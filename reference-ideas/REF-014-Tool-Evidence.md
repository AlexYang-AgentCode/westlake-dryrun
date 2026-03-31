# REF-014: 工具输出即证据模式 ⭐⭐⭐

## 来源
- **项目**: agentic-flow, 17.1.111
- **源码位置**: 17.Matrix/17.0.TheOracle/evolution/evolution_validation.md
- **原始实现**: Docker容器内运行验证，输出JSON结果

## 核心机理

所有验证结果必须来自工具输出，不来自Agent的自我评价：

```json
// evidence/case-001/result.json
{
  "summary": {
    "total": 150,
    "passed": 142,
    "failed": 8,
    "passRate": "94.67%"
  },
  "results": [
    {
      "test": "integer_division",
      "status": "FAIL",
      "error": "expected 3, got 3.33"
    }
  ],
  "metadata": {
    "tool": "jest",
    "version": "29.7.0",
    "timestamp": "2026-03-15T10:00:00Z",
    "duration_ms": 1250
  }
}
```

**为什么有效：**
```
❌ Agent说"通过了" → 可能是幻觉
✅ 工具输出JSON"passed": 142 → 来自编译器/测试框架，无法编造
```

**证据链：**
```
Agent生成代码
  ↓
编译器运行（gcc/npx tsc）
  ↓
输出写入evidence/result.json
  ↓
Orchestrator读取JSON判断结果
  ↓
Agent无法篡改结果（只读权限）
```

## 对"极少烧ROM，持续自我进化"的价值

### 1. 防伪
```
场景：Agent声称"Adapter工作正常"

❌ 软性验证：Agent自我报告 → 可能掩盖问题
✅ 工具输出：实际运行测试 → 失败细节在JSON中
```

### 2. 可审计
```
evidence/
├── case-001/
│   ├── 20260315-100000/      # 第一次尝试
│   │   ├── result.json       # 验证结果
│   │   ├── compile.log       # 编译输出
│   │   └── screenshot.png    # UI对比截图
│   └── 20260315-100500/      # 第二次尝试（修复后）
│       └── ...
```

### 3. 结构化分析
```typescript
// 从证据中提取模式
const failures = loadAllEvidence()
  .filter(e => !e.passed)
  .map(e => ({
    error: e.results[0]?.error,
    tool: e.metadata.tool,
    timestamp: e.metadata.timestamp
  }));

// 分析最常见失败原因
const topFailures = countBy(failures, f => f.error?.substring(0, 30));
```

## 适用场景

| 场景 | 工具 | 输出 |
|------|------|------|
| 编译检查 | tsc/gcc | 错误列表、退出码 |
| 单元测试 | jest/pytest | 通过/失败、覆盖率 |
| UI对比 | puppeteer + pixelmatch | SSIM分数、diff图 |
| 性能测试 | k6/benchmark.js | 延迟、吞吐量 |
| 安全扫描 | bandit/semgrep | 漏洞列表、CVSS分数 |

## 实施建议

### 最小实现（Day 1）
```bash
#!/bin/bash
# run-and-evidence.sh

CASE_ID="$1"
TIMESTAMP=$(date +%Y%m%d-%H%M%S)
EVIDENCE_DIR="evidence/${CASE_ID}/${TIMESTAMP}"
mkdir -p "$EVIDENCE_DIR"

# 运行编译
npx tsc --noEmit "adapters/${CASE_ID}.ts" 2> "${EVIDENCE_DIR}/compile.log"
COMPILE_EXIT=$?

# 运行测试
npx jest "tests/${CASE_ID}.test.ts" --json > "${EVIDENCE_DIR}/test-result.json" 2>/dev/null
TEST_EXIT=$?

# 生成结构化证据
cat > "${EVIDENCE_DIR}/result.json" <<EOF
{
  "case_id": "${CASE_ID}",
  "timestamp": "$(date -Iseconds)",
  "compile": {
    "passed": $([ $COMPILE_EXIT -eq 0 ] && echo true || echo false),
    "exit_code": $COMPILE_EXIT
  },
  "test": {
    "passed": $([ $TEST_EXIT -eq 0 ] && echo true || echo false),
    "exit_code": $TEST_EXIT
  },
  "overall": $([ $COMPILE_EXIT -eq 0 ] && [ $TEST_EXIT -eq 0 ] && echo '"PASS"' || echo '"FAIL"')
}
EOF

echo "Evidence saved to ${EVIDENCE_DIR}/result.json"
```

### 进阶实现（Week 1）
```typescript
interface Evidence {
  caseId: string;
  timestamp: string;
  tool: string;
  toolVersion: string;
  results: TestResult[];
  summary: {
    total: number;
    passed: number;
    failed: number;
    passRate: string;
  };
  artifacts: {
    logs?: string;
    screenshots?: string[];
    traces?: string;
  };
  metadata: {
    durationMs: number;
    environment: string;
  };
}

class EvidenceCollector {
  async runAndCollect(
    caseId: string,
    runner: () => Promise<TestOutput>
  ): Promise<Evidence> {
    const startTime = Date.now();
    const output = await runner();
    const duration = Date.now() - startTime;

    const evidence: Evidence = {
      caseId,
      timestamp: new Date().toISOString(),
      tool: output.tool,
      toolVersion: output.version,
      results: output.results,
      summary: {
        total: output.results.length,
        passed: output.results.filter(r => r.status === 'PASS').length,
        failed: output.results.filter(r => r.status === 'FAIL').length,
        passRate: calculatePassRate(output.results)
      },
      artifacts: {
        logs: output.logPath,
        screenshots: output.screenshots
      },
      metadata: {
        durationMs: duration,
        environment: process.env.NODE_ENV || 'development'
      }
    };

    await this.saveEvidence(evidence);
    return evidence;
  }

  async saveEvidence(evidence: Evidence): Promise<void> {
    const dir = `evidence/${evidence.caseId}/${formatTimestamp(evidence.timestamp)}`;
    await fs.mkdir(dir, { recursive: true });
    await fs.writeFile(
      `${dir}/result.json`,
      JSON.stringify(evidence, null, 2)
    );
  }
}
```

## 参考代码片段

```typescript
// From 17.Matrix/17.0.TheOracle/15.5-E2ETest/verify-agent.ts
/**
 * 工具输出即证据 —— 验证结果来自test-runner，不是Agent自述
 */
async verify(caseId: string): Promise<VerificationResult> {
  // 运行测试工具
  const result = await runTestCase(caseId);

  const verification: VerificationResult = {
    caseId,
    passed: result.verdict === 'PASS',
    attempt: result.attempt,
    diff: result.diff,
    androidOutput: result.android_output,
    harmonyOutput: result.harmony_raw_output,
    adapterOutput: result.adapter_output,
  };

  // 输出结构化结果（不可篡改）
  return verification;
}
```

```json
// 示例证据输出
{
  "summary": { "total": 150, "passed": 142, "failed": 8, "passRate": "94.67%" },
  "results": [
    {"test": "integer_division", "status": "FAIL", "error": "expected 3, got 3.33"}
  ]
}
```

## 关联参考
- REF-011: 退出码阻断验证
- REF-007: 角色隔离验证
