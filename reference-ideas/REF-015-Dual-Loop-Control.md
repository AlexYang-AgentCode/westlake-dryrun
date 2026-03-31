# REF-015: 双循环控制模式 ⭐⭐

## 来源
- **框架**: Magnetic-One
- **源码位置**: 17.Matrix/17.0.TheOracle/TianDao-Interrogation.md
- **原始实现**: Magnetic-One 的外循环战略 + 内循环战术

## 核心机理

外循环战略重规划 + 内循环战术调度：

```
┌─────────────────────────────────────────────┐
│  外循环 (Outer Loop) — 战略层                │
│  ├─ 频率: 每N轮或停滞检测触发                │
│  ├─ 职责: 重规划策略、调整目标、资源分配      │
│  └─ 输出: 新的Plan、更新的优先级             │
└──────────────┬──────────────────────────────┘
               │ 提供Plan
┌──────────────▼──────────────────────────────┐
│  内循环 (Inner Loop) — 战术层                │
│  ├─ 频率: 每轮执行                           │
│  ├─ 职责: 执行当前Plan、案例处理、规则应用    │
│  └─ 输出: 案例结果、进展报告                  │
└──────────────┬──────────────────────────────┘
               │ 反馈进展/停滞
               └──────────────────────────────┘
```

**对比单循环：**
```
单循环：
  处理案例A → 处理案例B → 发现案例B停滞 → 继续硬磕 → 浪费时间

双循环：
  内循环: 处理案例A → 处理案例B → 报告停滞
  外循环: 检测到停滞 → 重规划: 跳过B，先处理C → 内循环执行新Plan
```

## 对"极少烧ROM，持续自我进化"的价值

### 1. 战略调整能力
```
外循环发现：
  - T1案例通过率95% → 可以并行加速
  - T4案例通过率30% → 需要暂停，分析模式
  ↓
调整策略：
  - 增加T1并行度
  - T4案例改为串行，每个详细分析
```

### 2. 资源优化
```
外循环评估：
  - 当前10个槽位，8个被T4案例占用（停滞中）
  - T1案例排队等待
  ↓
重规划：
  - 释放4个T4案例到队列
  - 4个槽位给T1案例
  - 整体吞吐量提升
```

### 3. 自适应策略
```
外循环分析历史：
  - 规则R01在80%案例有效
  - 规则R02只在20%案例有效
  ↓
内循环调整：
  - 新案例优先尝试R01
  - R02只在特定场景应用
```

## 适用场景

| 场景 | 外循环职责 | 内循环职责 |
|------|-----------|-----------|
| 批量Adapter | 重规划优先级、调整并行度 | 处理单个案例、应用规则 |
| 规则发现 | 决定发现方向、验证策略 | 提取模式、测试候选规则 |
| 性能优化 | 调整资源分配、识别瓶颈 | 执行优化、测量指标 |
| 故障恢复 | 决策重试/跳过/ESCALATE | 执行重试、收集错误信息 |

## 实施建议

### 最小实现（Day 1）
```typescript
// 简单的双循环
class DualLoopSystem {
  private innerLoopCount = 0;
  private outerLoopInterval = 10;  // 每10轮外循环检查

  async run(): Promise<void> {
    while (!this.isComplete()) {
      // 外循环：战略调整
      if (this.innerLoopCount % this.outerLoopInterval === 0) {
        console.log('[Outer Loop] Replanning...');
        await this.strategicReplan();
      }

      // 内循环：战术执行
      console.log('[Inner Loop] Processing cases...');
      await this.tacticalExecute();

      this.innerLoopCount++;
    }
  }

  private async strategicReplan(): Promise<void> {
    const stats = this.calculateStats();

    // 根据统计数据调整策略
    if (stats.stallRate > 0.3) {
      console.log('  High stall rate detected, reducing parallelism');
      this.reduceParallelism();
    }

    if (stats.t1PassRate > 0.9) {
      console.log('  T1 cases performing well, increasing batch size');
      this.increaseBatchSize();
    }
  }

  private async tacticalExecute(): Promise<void> {
    const nextCases = this.selectNextCases();
    await Promise.all(nextCases.map(c => this.processCase(c)));
  }
}
```

### 进阶实现（Week 1）
```typescript
interface LoopContext {
  iteration: number;
  stats: SystemStats;
  history: LoopHistory[];
}

interface ReplanDecision {
  action: 'continue' | 'adjust' | 'pivot' | 'escalate';
  newStrategy?: Strategy;
  reason: string;
}

class OuterLoop {
  private stagnationThreshold = 3;  // 3轮无进展触发重规划

  async evaluate(context: LoopContext): Promise<ReplanDecision> {
    // 检查停滞
    if (this.isStagnating(context)) {
      return {
        action: 'adjust',
        newStrategy: this.generateNewStrategy(context),
        reason: `Stagnation detected for ${this.stagnationThreshold} rounds`
      };
    }

    // 检查阶段性目标完成
    if (this.isPhaseComplete(context)) {
      return {
        action: 'pivot',
        newStrategy: this.nextPhaseStrategy(context),
        reason: 'Phase objectives met, moving to next phase'
      };
    }

    return { action: 'continue', reason: 'Progress on track' };
  }

  private isStagnating(context: LoopContext): boolean {
    const recentHistory = context.history.slice(-this.stagnationThreshold);
    return recentHistory.every(h => h.progress < 0.05);  // 连续多轮进展<5%
  }

  private generateNewStrategy(context: LoopContext): Strategy {
    const stats = context.stats;

    // 基于当前状态生成新策略
    if (stats.failedCases > stats.solvedCases * 0.5) {
      return {
        type: 'focus_mode',
        focusOn: 'hard_cases',
        resources: { slots: 5, maxRetries: 10 }
      };
    }

    return {
      type: 'throughput_mode',
      focusOn: 'easy_cases',
      resources: { slots: 20, maxRetries: 3 }
    };
  }
}

class InnerLoop {
  constructor(private outerLoop: OuterLoop) {}

  async run(context: LoopContext): Promise<LoopResult> {
    // 执行当前策略
    const cases = this.selectCases(context.currentStrategy);
    const results = await this.processBatch(cases);

    // 更新上下文
    context.stats = this.updateStats(context.stats, results);
    context.history.push({
      iteration: context.iteration,
      progress: this.calculateProgress(results),
      casesProcessed: cases.length
    });

    // 触发外循环评估
    const decision = await this.outerLoop.evaluate(context);

    if (decision.action !== 'continue') {
      context.currentStrategy = decision.newStrategy || context.currentStrategy;
      console.log(`[Strategy Change] ${decision.reason}`);
    }

    return { results, context };
  }
}
```

## 参考代码片段

```typescript
// From 17.Matrix/17.0.TheOracle/engine/self-run.ts
// SelfRunningLoop 简化版双循环
class SelfRunningLoop {
  private maxRetries = 5;

  async run(maxIterations = 10): Promise<void> {
    for (let i = 0; i < maxIterations; i++) {
      console.log(`\n>>> Iteration ${i + 1}/${maxIterations} <<<`);

      // 外循环检查：是否全部完成
      if (this.workflow.isAllComplete()) {
        console.log('All cases completed!');
        return;
      }

      // 内循环：选择并处理案例
      const nextCase = this.workflow.getNextCase();
      if (!nextCase) {
        console.log('No cases to process');
        break;
      }

      await this.processCase(nextCase.id);
    }
  }

  // 处理单个案例（内循环）
  async processCase(caseId: string): Promise<boolean> {
    for (let attempt = 1; attempt <= this.maxRetries; attempt++) {
      this.bridge.generate(caseId);
      const result = await this.tester.verify(caseId);

      if (result.passed) return true;

      // 战术调整：学习新规则
      const newRule = this.tester.extractRule(result);
      if (newRule) console.log(`Discovered: ${newRule}`);
    }

    // 战略决策：放弃并上报
    console.log('Escalating to human...');
    return false;
  }
}
```

## 关联参考
- REF-001: Progress Ledger
- REF-005: Superstep并行
