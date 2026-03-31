# REF-006: Handoff 自主移交模式 ⭐

## 来源
- **框架**: AutoGen
- **源码位置**: 17.Matrix/17.0.TheOracle/TianDao-Interrogation.md
- **原始实现**: AutoGen 的 Swarm 模式

## 核心机理

Agent 自主判断何时将任务移交给更合适的 Agent：

```python
# AutoGen Swarm 伪代码
class BridgeAgent(Agent):
    def handle(self, task):
        if task.type in self.capabilities:
            return self.process(task)

        # 自主判断移交
        if task.type == "ui_component":
            return handoff_to("ViewBridgeAgent", task)
        elif task.type == "lifecycle":
            return handoff_to("ActivityBridgeAgent", task)
        else:
            return handoff_to("Orchestrator", task)  # 未知类型上报

# 移交协议
handoff_context = {
    "from": "BridgeAgent",
    "to": "ViewBridgeAgent",
    "reason": "UI组件转换超出当前能力范围",
    "task_summary": "将Android Button转换为Harmony Button",
    "partial_result": {...},  # 已完成的分析结果
    "timestamp": "2026-03-29T10:00:00Z"
}
```

**vs 中央调度：**
```
中央调度模式：Orchestrator决定谁做什么（中心化）
Handoff模式：Agent自己决定干不了时交给谁（去中心化）
```

## 对"极少烧ROM，持续自我进化"的价值

### 1. 降低Orchestrator复杂度
```typescript
// 中央调度：Orchestrator必须知道所有Agent的能力和状态
orchestrator.route(task); // 复杂的路由逻辑

// Handoff：Orchestrator只处理无法分发的任务
agent.process(task); // Agent自己决定，处理不了再上报
```

### 2. 动态能力扩展
```typescript
// 新Bridge加入时，只需声明自己的能力
class NotificationBridgeAgent {
  capabilities = ['notification', 'push', 'alarm'];

  canHandle(task: Task): boolean {
    return this.capabilities.includes(task.category);
  }

  handle(task: Task): Result {
    if (!this.canHandle(task)) {
      return this.handoffTo(findAgentWithCapability(task.category));
    }
    return this.process(task);
  }
}
```

### 3. 负载均衡
```typescript
// BridgeAgent 发现队列过长时主动移交
if (this.pendingTasks.length > 5) {
  return this.handoffTo(
    findLeastBusyAgent(this.capabilities),
    task,
    { reason: 'load_balancing' }
  );
}
```

## 适用场景

| 场景 | Handoff触发条件 |
|------|----------------|
| API类型不匹配 | Bridge遇到非自己领域的API |
| 复杂度超限 | 案例复杂度超过当前Bridge处理能力 |
| 队列拥堵 | 当前Agent待处理任务过多 |
| 新规则发现 | 需要其他Bridge协作验证 |

## 实施建议

### 最小实现（Day 1）
```typescript
// 简单的移交协议
interface HandoffMessage {
  type: 'handoff';
  from: string;
  to: string;
  task: Task;
  reason: string;
  context: any;
}

class BridgeAgent {
  private capabilities: string[];

  async process(task: Task): Promise<Result> {
    if (!this.canHandle(task)) {
      // 自主移交
      const targetAgent = this.findCapableAgent(task.type);
      return this.handoff(targetAgent, task);
    }
    // 正常处理
    return this.doProcess(task);
  }

  private canHandle(task: Task): boolean {
    return this.capabilities.includes(task.category);
  }

  private async handoff(target: string, task: Task): Promise<Result> {
    console.log(`[Handoff] ${this.name} -> ${target}: ${task.id}`);
    return messageBus.send(target, {
      type: 'handoff',
      from: this.name,
      to: target,
      task,
      reason: 'capability_mismatch',
      context: this.getPartialResult(task)
    });
  }
}
```

### 进阶实现（Week 1）
```typescript
// Handoff路由器
class HandoffRouter {
  private agents: Map<string, AgentCapabilities> = new Map();

  registerAgent(agentId: string, capabilities: string[]): void {
    this.agents.set(agentId, { id: agentId, capabilities });
  }

  findBestAgent(task: Task, exclude: string[] = []): string | null {
    const candidates = Array.from(this.agents.entries())
      .filter(([id, caps]) =>
        !exclude.includes(id) &&
        caps.capabilities.includes(task.category)
      );

    // 选择负载最低的
    return candidates
      .sort((a, b) => a[1].load - b[1].load)[0]?.[0] || null;
  }

  async routeHandoff(handoff: HandoffMessage): Promise<void> {
    const target = this.findBestAgent(handoff.task, [handoff.from]);
    if (!target) {
      // 无合适Agent，上报Orchestrator
      await this.escalate(handoff);
      return;
    }
    await this.sendToAgent(target, handoff);
  }
}
```

## 参考代码片段

```typescript
// From 17.Matrix/17.0.TheOracle/15.1-WorkFlow/workflow-agent.ts
// WorkFlow分配任务（简化版handoff）
assignToBridge(caseId: string): void {
  const task = {
    type: 'ADAPTER_GENERATION',
    case_id: caseId,
    assigned_at: new Date().toISOString(),
    status: 'PENDING',
  };

  // 写入任务文件，Bridge自主拉取
  fs.writeFileSync(taskFile, JSON.stringify(task, null, 2));
  console.log(`[WorkFlow] Assigned case ${caseId} to Bridge Agent`);
}
```

## 关联参考
- REF-001: Progress Ledger
- REF-005: Superstep并行
