# REF-003: JSON Schema 契约验证模式 ⭐⭐

## 来源
- **框架**: LangGraph
- **源码位置**: 17.Matrix/17.0.TheOracle/TianDao-Interrogation.md
- **原始实现**: LangGraph 的 TypedDict 状态定义

## 核心机理

用 JSON Schema 作为需求契约，编译期/运行时强制校验：

```python
# LangGraph TypedDict 风格
from typing import TypedDict

class AdapterState(TypedDict):
    case_id: str
    android_output: list[int]  # 必须匹配此类型
    harmony_output: list[int]
    adapter_code: str
    status: Literal["pending", "pass", "fail"]

# 任何状态变更必须满足此Schema
# 否则在编译期（TypeScript）或运行期（Python）报错
```

**核心思想：类型即契约**
- Schema变更 = 需求变更
- 违反Schema = 编译/运行错误（无法绕过）
- Agent无法悄悄写一个不符合规范的YAML

## 对"极少烧ROM，持续自我进化"的价值

### 1. 防止契约漂移
```yaml
# 无Schema时：Agent可能输出任意格式
output: {a: 1}  # 这是啥？

# 有Schema时：必须符合预定义结构
output:
  case_id: "101"
  android_output: [1, 2, 3]
  harmony_output: [0, 1, 2]
  adapter_code: "..."
  status: "pass"  # 只能是 pending/pass/fail
```

### 2. 自动化验证生成
```typescript
// 从JSON Schema自动生成验证函数
const schema = {
  type: "object",
  required: ["case_id", "status"],
  properties: {
    case_id: { type: "string", pattern: "^\\d{3}$" },
    status: { enum: ["pending", "pass", "fail"] }
  }
};

// 自动生成
function validateOutput(output: unknown): boolean {
  return ajv.validate(schema, output); // true/false
}
```

### 3. 版本化契约
```yaml
# api-contract-v1.yaml
version: "1.0"
definitions:
  SystemAPI:
    getSequence:
      input: { n: integer }
      output: { items: integer[], startIndex: 1 }  # 安卓从1开始

# api-contract-v2.yaml
version: "2.0"
definitions:
  SystemAPI:
    getSequence:
      input: { n: integer, offset?: integer }      # 新增可选参数
      output: { items: integer[], startIndex: 1 }
```

## 适用场景

| 场景 | Schema应用 |
|------|-----------|
| API行为契约 | 输入参数、输出格式、边界条件 |
| Adapter输出 | 代码结构、函数签名、导出模块 |
| 规则定义 | 规则ID、模式、置信度、验证案例 |
| 验证结果 | 通过/失败、diff详情、截图路径 |

## 实施建议

### 最小实现（Day 1）
```yaml
# api-contract.yaml — 人工编写
SystemAPI:
  getSequence:
    signature: "getSequence(n: number): number[]"
    behavior:
      input: { n: 5 }
      expected_output: [1, 2, 3, 4, 5]  # Ground Truth
      constraints:
        - "output.length === n"
        - "output[0] === 1"  # 安卓从1开始
```

### 进阶实现（Week 1）
```typescript
// 用Zod定义Schema并运行时校验
import { z } from 'zod';

const AdapterOutputSchema = z.object({
  caseId: z.string().regex(/^\d{3}$/),
  androidOutput: z.array(z.number()),
  harmonyOutput: z.array(z.number()),
  adapterCode: z.string().min(1),
  status: z.enum(['pending', 'pass', 'fail']),
  diff: z.array(z.object({
    path: z.string(),
    expected: z.any(),
    actual: z.any()
  })).optional()
});

// 运行时校验
try {
  const validated = AdapterOutputSchema.parse(rawOutput);
  // 通过
} catch (e) {
  // Schema违规，阻断
  throw new Error(`Schema violation: ${e.message}`);
}
```

## 参考代码片段

```yaml
# From 17.Matrix/17.0.TheOracle/15.2-AppHouse/output/api-manifest.yaml
api_manifest:
  version: "1.0"
  apis:
    - name: "getSequence"
      signature: "getSequence(int n): int[]"
      contract:
        input: { n: 5 }
        expected_output: [1, 2, 3, 4, 5]
        behavior_notes:
          - "Returns array of length n"
          - "First element is 1 (1-indexed)"
          - "Each subsequent element increments by 1"
```

## 关联参考
- REF-002: Guardrail函数链
- REF-007: 角色隔离验证
