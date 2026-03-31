# REF-001: Progress Ledger 进展追踪模式 ⭐⭐⭐

## 来源
- **框架**: Magnetic-One (Microsoft)
- **源码位置**: 17.Matrix/17.0.TheOracle/TianDao-Interrogation.md
- **原始实现**: Magnetic-One 的 Orchestrator 组件

## 核心机理

Progress Ledger 是一个显式的进展追踪机制，Agent 每轮必须回答：
1. **我们在循环吗？** —— 检测是否陷入重复模式
2. **有进展吗？** —— 对比本轮与上轮产出差异
3. **停滞了吗？** —— 连续多轮无进展触发重规划

```typescript
// 简化实现逻辑
interface ProgressLedger {
  round: number;
  actions: Action[];
  outcomes: Outcome[];
  progressScore: number; // 0-100
  isStalled: boolean;    // 连续3轮progressScore < 10
}

// 每轮调度前评估
function evaluateProgress(ledger: ProgressLedger): boolean {
  const recentRounds = ledger.outcomes.slice(-3);
  const avgProgress = recentRounds.reduce((a, b) => a + b.progress, 0) / 3;

  if (avgProgress < 10) {
    ledger.isStalled = true;
    return false; // 需要重规划
  }
  return true;
}
```

## 对"极少烧ROM，持续自我进化"的价值

### 1. 防止假进展
- 问题：Agent 看似在工作，实际在原地打转
- 解法：量化进展分数，低于阈值自动触发重规划

### 2. 动态资源分配
- 停滞案例自动降低优先级，释放计算资源
- 高进展案例增加重试次数上限

### 3. 自我进化触发器
```
stall_count > 3 → 触发规则学习
stall_count > 5 → 触发架构调整
stall_count > 10 → ESCALATE 到人类
```

## 适用场景

| 场景 | 应用方式 |
|------|----------|
| Adapter批量转换 | 每案例维护progress ledger，停滞自动跳过 |
| 规则发现循环 | 检测是否发现新规则，无进展则换案例 |
| 多Agent协作 | Orchestrator用ledger判断哪个Agent卡住了 |

## 实施建议

### 最小实现（Day 1）
```json
// stats.json 增加字段
{
  "case_001": {
    "attempts": 3,
    "last_progress": "discovered R01", // 上次进展
    "stall_rounds": 0,                  // 停滞轮数
    "status": "active"
  }
}
```

### 进阶实现（Week 1）
```typescript
// ProgressLedger 类
class ProgressLedger {
  recordAttempt(caseId: string, outcome: Outcome): void {
    const history = this.getHistory(caseId);
    const progress = this.calculateProgress(outcome, history.lastOutcome);

    if (progress < 0.1) {
      history.stallCount++;
      if (history.stallCount >= 3) {
        this.triggerReplan(caseId);
      }
    } else {
      history.stallCount = 0;
    }
  }
}
```

## 参考代码片段

```typescript
// From 17.Matrix/17.0.TheOracle/15.1-WorkFlow/workflow-agent.ts
getNextCase(): TestCase | null {
  // 优先级计算考虑了失败次数（类似stall检测）
  unsolvedCases.push({
    case: testCase,
    priority: testCase.tier * 1000 - history.attempts.length
  });
}
```

## 关联参考
- REF-015: 多循环控制（外循环+内循环）
- REF-009: 规则发现与学习
