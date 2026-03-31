# REF-002: Guardrail 函数链验证模式 ⭐⭐⭐

## 来源
- **框架**: CrewAI
- **源码位置**: 17.Matrix/17.0.TheOracle/TianDao-Interrogation.md
- **原始实现**: CrewAI 的 Task 组件

## 核心机理

Guardrail 是可编程的输出校验函数链，每个 Task 可挂载多个验证函数：

```python
# CrewAI 风格伪代码
@task
def generate_adapter(case):
    return bridge.generate(case)

@guardrail(generate_adapter)
def check_compiles(output):
    result = compile(output.code)
    return result.success  # 失败则触发重试

@guardrail(generate_adapter)
def check_output_format(output):
    return validate_schema(output, AdapterSchema)  # JSON Schema校验

@guardrail(generate_adapter)
def check_no_placeholder(output):
    return "TODO" not in output.code and "FIXME" not in output.code
```

**失败处理逻辑：**
```
验证函数返回 False → 触发重试（retry_count++）
retry_count > max_retries(5) → raise 并 ESCALATE
```

## 对"极少烧ROM，持续自我进化"的价值

### 1. 可组合的质量门禁
- 不同案例可配置不同的guardrail组合
- 基础案例：只检查编译通过
- 关键案例：额外检查行为一致性、安全漏洞

### 2. 自动化规则发现
```typescript
// Guardrail 失败模式 → 自动发现规则
if (guardrail.failureType === 'compile_error') {
  const pattern = extractErrorPattern(error);
  ruleCandidate = {
    pattern: pattern,
    fix: generateFix(pattern),
    confidence: 0.7
  };
}
```

### 3. 渐进式收紧
```yaml
# T1案例配置（宽松）
guardrails:
  - file_exists
  - syntax_valid

# T4案例配置（严格）
guardrails:
  - file_exists
  - syntax_valid
  - behavior_match
  - visual_ssim_0.95
  - security_scan
```

## 适用场景

| 场景 | Guardrail 示例 |
|------|----------------|
| Adapter代码生成 | 编译检查、单元测试通过、无TODO |
| 规则发现 | 输出符合YAML Schema、无重复规则ID |
| E2E验证 | 截图对比SSIM>0.95、输出匹配期望值 |
| 知识入库 | JSON Schema校验、无冲突规则 |

## 实施建议

### 最小实现（Day 1）
```bash
# verify-gate.sh 作为单一guardrail
#!/bin/bash
set -e

# Guard 1: 文件存在
[ -f "$OUTPUT" ] || { echo "FAIL: file not found"; exit 2; }

# Guard 2: 不为空
[ -s "$OUTPUT" ] || { echo "FAIL: file empty"; exit 2; }

# Guard 3: 语法检查（可选）
npx tsc --noEmit "$OUTPUT" 2>/dev/null || { echo "FAIL: syntax error"; exit 2; }

echo "PASS"
exit 0
```

### 进阶实现（Week 1）
```typescript
// Guardrail 链式配置
interface GuardrailConfig {
  name: string;
  check: (output: any) => Promise<boolean>;
  onFail: 'retry' | 'escalate' | 'skip';
  maxRetries: number;
}

const guardrails: GuardrailConfig[] = [
  { name: 'compile', check: compileCheck, onFail: 'retry', maxRetries: 5 },
  { name: 'behavior', check: behaviorCheck, onFail: 'retry', maxRetries: 3 },
  { name: 'security', check: securityCheck, onFail: 'escalate', maxRetries: 0 }
];

async function runWithGuardrails(
  task: () => Promise<Output>,
  guardrails: GuardrailConfig[]
): Promise<Output> {
  for (let attempt = 1; attempt <= MAX_ATTEMPTS; attempt++) {
    const output = await task();

    for (const guard of guardrails) {
      const pass = await guard.check(output);
      if (!pass) {
        if (guard.onFail === 'escalate') throw new Error('ESCALATE');
        if (attempt >= guard.maxRetries) throw new Error('MAX_RETRY_EXCEEDED');
        break; // 重试
      }
    }
    return output; // 全部通过
  }
}
```

## 参考代码片段

```typescript
// From 17.Matrix/17.0.TheOracle/engine/self-run.ts
// SelfRunningLoop 中的maxRetries机制
async processCase(caseId: string): Promise<boolean> {
  for (let attempt = 1; attempt <= this.maxRetries; attempt++) {
    this.bridge.generate(caseId);
    const result = await this.tester.verify(caseId);

    if (result.passed) return true; // Guard通过

    // Guard失败 → 学习新规则 → 重试
    const newRule = this.tester.extractRule(result);
    if (newRule) console.log(`Discovered: ${newRule}`);
  }
  return false; // 所有guard都失败，ESCALATE
}
```

## 关联参考
- REF-011: 退出码阻断验证
- REF-010: 分层门禁验证
