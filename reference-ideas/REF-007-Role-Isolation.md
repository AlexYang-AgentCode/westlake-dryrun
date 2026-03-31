# REF-007: 角色隔离验证模式 ⭐⭐⭐

## 来源
- **框架**: MetaGPT
- **源码位置**: 17.Matrix/17.0.TheOracle/TianDao-Interrogation.md
- **原始实现**: MetaGPT 的多角色协作流程

## 核心机理

写代码的人 ≠ 写测试的人 ≠ 验证的人：

```python
# MetaGPT 角色定义
class Engineer(Role):
    """写代码的工程师"""
    def __init__(self):
        self.actions = [WriteDesign, WriteCode]
        self.watch([UserRequirement])  # 监听需求

class QaEngineer(Role):
    """写测试的QA工程师"""
    def __init__(self):
        self.actions = [WriteTest, RunTest]
        self.watch([WriteCode])  # 监听代码完成

class CodeReviewer(Role):
    """代码审查者"""
    def __init__(self):
        self.actions = [ReviewCode]
        self.watch([WriteCode, WriteTest])

# 关键：QA看不到Engineer的设计文档，只看代码输出
# Reviewer看不到具体实现思路，只看代码和测试
```

**信息隔离原则：**
1. **需求隔离**：QA只从需求出发写测试，不看Engineer的设计
2. **代码隔离**：Reviewer独立审查代码质量
3. **验证隔离**：Validator检查需求覆盖度，不看实现细节

## 对"极少烧ROM，持续自我进化"的价值

### 1. 防止自我验证
```
❌ 错误模式：
BridgeAgent生成Adapter → BridgeAgent自己验证 → "PASS"

✅ 正确模式：
BridgeAgent生成Adapter
  ↓ 信息隔离（看不到代码）
E2ETestAgent独立验证 → "FAIL: 输出不匹配"
  ↓
BridgeAgent分析diff → 改进代码
  ↓
E2ETestAgent独立验证 → "PASS"
```

### 2. 发现隐藏假设
```typescript
// BridgeAgent的隐藏假设："鸿蒙数组从0开始"
// QaEngineer的测试：getSequence(5) 应该返回 [1,2,3,4,5]
// 验证结果：实际 [0,1,2,3,4] → 发现偏移问题

// 如果没有角色隔离，BridgeAgent可能不会测试边界值
```

### 3. 质量制衡
```
Engineer追求：代码简洁、实现优雅
QaEngineer追求：测试覆盖、边界情况
CodeReviewer追求：可读性、可维护性

三方制衡 → 避免单方面优化导致整体质量下降
```

## 适用场景

| 场景 | 角色分配 |
|------|----------|
| Adapter生成 | BridgeAgent写代码，E2ETestAgent验证 |
| 规则发现 | RuleDiscoverAgent发现，RuleValidatorAgent验证 |
| 复杂案例 | DesignerAgent设计，ImplementerAgent实现，VerifierAgent验证 |
| 代码重构 | RefactorAgent重构，RegressionTestAgent验证 |

## 实施建议

### 最小实现（Day 1）
```bash
# 文件系统隔离
# BridgeAgent 只能读写 adapters/ 目录
# E2ETestAgent 只能读写 evidence/ 目录

# 验证流程
echo "[BridgeAgent] 生成Adapter..."
node bridge-agent.js case_001
# 输出到 adapters/case_001.ts

echo "[E2ETestAgent] 独立验证..."
# E2ETestAgent 看不到 adapters/case_001.ts 的内容
# 只能从 api-contract.yaml 读取期望输出
node e2e-test.js case_001
# 输出到 evidence/case_001/result.json
```

### 进阶实现（Week 1）
```typescript
// 角色权限管理
interface Role {
  name: string;
  readPaths: string[];   // 可读目录
  writePaths: string[];  // 可写目录
  canSeeCode: boolean;   // 是否能看到代码
}

const roles: Role[] = [
  {
    name: 'BridgeAgent',
    readPaths: ['knowledge/', 'cases/'],
    writePaths: ['adapters/'],
    canSeeCode: true
  },
  {
    name: 'E2ETestAgent',
    readPaths: ['api-contract/', 'adapters/'],  // 只能读最终代码，不能看生成过程
    writePaths: ['evidence/'],
    canSeeCode: false  // 黑盒验证
  },
  {
    name: 'RuleValidator',
    readPaths: ['knowledge/rules.yaml'],
    writePaths: ['knowledge/validated-rules.yaml'],
    canSeeCode: false
  }
];

// 验证器检查权限
function verifyIsolation(agent: Agent, action: Action): boolean {
  const role = getRole(agent.role);

  if (action.type === 'read') {
    return role.readPaths.some(p => action.path.startsWith(p));
  }

  if (action.type === 'write') {
    return role.writePaths.some(p => action.path.startsWith(p));
  }

  return false;
}
```

## 参考代码片段

```typescript
// From 17.Matrix/17.0.TheOracle/15.5-E2ETest/verify-agent.ts
/**
 * 关键：本 Agent 对 Bridge Agent 不可见（黑盒测试）
 * E2ETestAgent 只看 api-contract 的期望输出
 * 不看 BridgeAgent 的 adapter 代码
 */
async verify(caseId: string): Promise<VerificationResult> {
  // 从 api-manifest 读取期望行为（Ground Truth）
  const contract = loadApiContract(caseId);

  // 运行测试（黑盒）
  const result = await runTestCase(caseId);

  // 对比实际输出与期望输出
  const passed = deepEqual(result.output, contract.expected_output);

  return { caseId, passed, diff: result.diff };
}
```

## 关联参考
- REF-011: 退出码阻断验证
- REF-014: 工具输出即证据
