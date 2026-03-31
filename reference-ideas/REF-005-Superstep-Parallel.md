# REF-005: Superstep 并行执行模式 ⭐⭐

## 来源
- **框架**: LangGraph
- **源码位置**: 17.Matrix/17.0.TheOracle/TianDao-Interrogation.md
- **原始实现**: LangGraph 的 Pregel 执行引擎

## 核心机理

Superstep模型：同一步骤内，所有**无依赖**节点并行执行：

```python
# LangGraph 伪代码
workflow = StateGraph(State)

# 定义并行节点
workflow.add_node("analyze_api_1", analyze_api_1)
workflow.add_node("analyze_api_2", analyze_api_2)
workflow.add_node("analyze_api_3", analyze_api_3)

# 设置边（无依赖关系 = 可并行）
workflow.add_edge(START, "analyze_api_1")
workflow.add_edge(START, "analyze_api_2")
workflow.add_edge(START, "analyze_api_3")

# 编译为 Pregel（支持Superstep）
app = workflow.compile()

# 执行：3个节点在同一Superstep并行执行
for event in app.stream(initial_state):
    print(event)  # 收到3个完成的信号
```

**依赖自动检测：**
```
Superstep 1: analyze_api_1, analyze_api_2, analyze_api_3（无依赖，并行）
  ↓
Superstep 2: generate_adapter（依赖analyze结果，等待全部完成）
  ↓
Superstep 3: verify_adapter_1, verify_adapter_2, verify_adapter_3（无依赖，并行）
```

## 对"极少烧ROM，持续自我进化"的价值

### 1. 最大化并行度
```typescript
// 传统串行：100个案例 × 30秒 = 3000秒 = 50分钟
// Superstep并行：100个案例 / 10槽位 × 30秒 = 300秒 = 5分钟

const batch = [
  'case_001', 'case_002', ..., 'case_100'
];

// 同一Tier的案例无依赖，可并行
await Promise.all(
  batch.map(caseId => processCase(caseId))
);
```

### 2. 动态负载均衡
```typescript
interface Superstep {
  nodes: string[];      // 当前可执行节点
  dependencies: Map<string, string[]>;  // 依赖图
}

function planSuperstep(state: State): Superstep {
  const completed = state.completedNodes;
  const ready = state.allNodes.filter(node =>
    node.dependencies.every(dep => completed.includes(dep))
  );
  return { nodes: ready, dependencies: state.dependencies };
}
```

### 3. 资源最优利用
```
场景：50个T1案例 + 30个T2案例 + 20个T3案例

Superstep 1: 50个T1案例并行（无依赖）
  ↓ 全部完成后
Superstep 2: 30个T2案例并行（可能依赖某些T1结果）
  ↓ 全部完成后
Superstep 3: 20个T3案例并行（可能依赖T1/T2结果）

vs 串行：必须等每个案例完全完成才能开始下一个
```

## 适用场景

| 场景 | 并行策略 |
|------|----------|
| 批量Adapter生成 | 同一Tier的案例并行 |
| 多API映射决策 | 无依赖的API并行分析 |
| 规则验证 | 已发现的规则并行验证 |
| E2E测试 | 独立的测试用例并行执行 |

## 实施建议

### 最小实现（Day 1）
```typescript
// 简单Promise.all并行
async function processBatch(caseIds: string[], concurrency: number): Promise<void> {
  const batches = chunk(caseIds, concurrency); // 分成每批N个

  for (const batch of batches) {
    // 同批并行执行
    await Promise.all(
      batch.map(id => processCase(id))
    );
  }
}
```

### 进阶实现（Week 1）
```typescript
// 依赖图 + Superstep调度
class SuperstepScheduler {
  private dependencyGraph: Map<string, Set<string>> = new Map();

  addNode(id: string, dependencies: string[] = []): void {
    this.dependencyGraph.set(id, new Set(dependencies));
  }

  getNextSuperstep(): string[] {
    const completed = this.getCompletedNodes();
    return Array.from(this.dependencyGraph.entries())
      .filter(([id, deps]) =>
        !this.isCompleted(id) &&
        Array.from(deps).every(dep => completed.includes(dep))
      )
      .map(([id]) => id);
  }

  async execute(): Promise<void> {
    while (this.hasPendingNodes()) {
      const superstep = this.getNextSuperstep();
      if (superstep.length === 0) {
        throw new Error('Deadlock detected');
      }

      console.log(`Superstep: ${superstep.join(', ')}`);
      await Promise.all(
        superstep.map(id => this.executeNode(id))
      );
    }
  }
}

// 使用示例
const scheduler = new SuperstepScheduler();

// T1案例无依赖
for (const caseId of t1Cases) {
  scheduler.addNode(caseId, []);
}

// T2案例依赖某些T1结果
for (const caseId of t2Cases) {
  scheduler.addNode(caseId, ['case_001', 'case_002']); // 示例依赖
}

await scheduler.execute();
```

## 参考代码片段

```typescript
// From 17.Matrix/17.0.TheOracle/engine/run-loop.sh
# 批处理循环（简化版Superstep）
BATCH_SIZE=5

for i in $(seq 101 199); do
  # 收集一批
  batch=""
  for j in $(seq 0 $((BATCH_SIZE-1))); do
    case_id=$((i + j))
    batch="$batch $case_id"
  done

  echo "Processing batch: $batch"
  # 并行处理（后台进程）
  for case_id in $batch; do
    npx tsx engine/self-run.ts "$case_id" &
  done
  wait  # 等待本批全部完成
  echo "Batch complete"
done
```

## 关联参考
- REF-001: Progress Ledger
- REF-014: 工具输出即证据
