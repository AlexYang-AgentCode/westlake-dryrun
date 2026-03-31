# REF-004: Checkpoint + Interrupt 断点续传模式 ⭐⭐

## 来源
- **框架**: LangGraph
- **源码位置**: 17.Matrix/17.0.TheOracle/TianDao-Interrogation.md
- **原始实现**: LangGraph 的 persistence 和 interrupt 机制

## 核心机理

在任何节点前后设置断点，人类审批后从断点恢复：

```python
# LangGraph 伪代码
workflow = StateGraph(AdapterState)

# 定义节点
workflow.add_node("analyze_api", analyze_api)
workflow.add_node("generate_adapter", generate_adapter)
workflow.add_node("verify_adapter", verify_adapter)

# 关键决策点设置 interrupt
workflow.add_node("mapping_decision", mapping_decision)

# 编译时设置 checkpoint 和 interrupt
app = workflow.compile(
    checkpointer=SqliteSaver("checkpoints.db"),  # 自动持久化
    interrupt_before=["mapping_decision"],       # 此节点前暂停
    interrupt_after=["verify_adapter"]           # 此节点后暂停
)

# 执行
for event in app.stream(initial_state):
    if event.get("__interrupt__"):
        # 暂停，等待人类输入
        human_input = await get_human_approval(event)
        # 恢复执行
        app.invoke(event, config={"input": human_input})
```

**核心特性：**
1. **自动持久化**：每个节点执行后自动保存状态到数据库
2. **精确恢复**：可以从任意checkpoint恢复，不丢失进度
3. **人类介入**：interrupt点暂停，人类审查后继续

## 对"极少烧ROM，持续自我进化"的价值

### 1. 安全的人类介入点
```
低风险操作（自动）
  ↓
API映射决策（interrupt → 人类确认）
  ↓
代码生成（自动）
  ↓
验证结果（interrupt → 人类审查失败原因）
  ↓
规则入库（interrupt → 人类确认新规则）
```

### 2. 容错与恢复
```typescript
// 系统崩溃后从checkpoint恢复
async function resumeFromCrash() {
  const checkpoints = await db.query(`
    SELECT * FROM checkpoints
    WHERE status = 'interrupted'
    ORDER BY timestamp DESC
  `);

  for (const cp of checkpoints) {
    console.log(`Resuming case ${cp.case_id} from ${cp.node}`);
    await workflow.resume(cp);
  }
}
```

### 3. 审计与回溯
```
checkpoint-001: analyze_api完成 → mapping_decision前（interrupt）
checkpoint-002: mapping_decision完成（人类批准: 使用R01） → generate_adapter前
checkpoint-003: generate_adapter完成 → verify_adapter前
checkpoint-004: verify_adapter完成（人类审查: PASS） → 归档
```

## 适用场景

| 场景 | Interrupt点设置 |
|------|----------------|
| API映射决策 | 策略选择前（Simulate vs Redirect） |
| 新规则发现 | 规则入库前（人工确认有效性） |
| 验证失败 | 自动修复5次后（人类决定放弃/继续） |
| 复杂案例 | Tier 3+ 案例验证后（人工抽查） |

## 实施建议

### 最小实现（Day 1）
```bash
# 用Git作为轻量级checkpoint
# 每个关键步骤后commit

# workflow.sh
cd workdir

# Step 1: 分析
echo "分析API..."
node analyze.js > output/analysis.json
git add . && git commit -m "[checkpoint] analyze complete"

# Step 2: 人工审查（interrupt）
echo "请审查 output/analysis.json，确认后继续"
read -p "按Enter继续..."

# Step 3: 生成
echo "生成Adapter..."
node generate.js > output/adapter.ts
git add . && git commit -m "[checkpoint] generate complete"
```

### 进阶实现（Week 1）
```typescript
// 状态持久化管理器
interface Checkpoint {
  id: string;
  caseId: string;
  node: string;           // 当前节点
  state: AdapterState;    // 完整状态快照
  status: 'running' | 'interrupted' | 'completed';
  timestamp: string;
}

class CheckpointManager {
  async save(checkpoint: Checkpoint): Promise<void> {
    await db.query(
      'INSERT INTO checkpoints VALUES (?, ?, ?, ?, ?, ?)',
      [checkpoint.id, checkpoint.caseId, checkpoint.node,
       JSON.stringify(checkpoint.state), checkpoint.status, checkpoint.timestamp]
    );
  }

  async resume(checkpointId: string, humanInput?: any): Promise<void> {
    const cp = await this.load(checkpointId);
    const workflow = this.getWorkflow(cp.caseId);

    // 恢复状态并继续
    await workflow.resume(cp.state, humanInput);
  }

  // 设置interrupt点
  shouldInterrupt(node: string): boolean {
    const interruptPoints = [
      'mapping_decision',
      'rule_confirmation',
      'escalation_point'
    ];
    return interruptPoints.includes(node);
  }
}
```

## 参考代码片段

```typescript
// From 17.Matrix/17.0.TheOracle/engine/self-run.ts
// SelfRunningLoop 中的escalate机制（简化版interrupt）
async processCase(caseId: string): Promise<boolean> {
  for (let attempt = 1; attempt <= this.maxRetries; attempt++) {
    // ... 尝试处理 ...

    if (attempt >= this.maxRetries) {
      console.log('  Escalating to human...'); // Interrupt点
      return false; // 暂停，等待人类
    }
  }
}
```

## 关联参考
- REF-001: Progress Ledger
- REF-013: 候选规则人工确认
