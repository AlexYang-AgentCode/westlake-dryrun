# REF-010: 分层门禁验证模式 ⭐⭐

## 来源
- **项目**: sub-agent, 17.1 多Agent项目
- **源码位置**: 17.Matrix/17.0.TheOracle/TianDao-Interrogation.md
- **原始实现**: MetaGPT的三级质量门

## 核心机理

三级质量门禁，每级有独立标准，通过才能进入下一阶段：

```
┌─────────────────────────────────────────┐
│  Gate 1: Planning (规划门禁)            │
│  标准: 需求覆盖率 ≥ 95%                  │
│  权重: 需求(25%) + 架构(20%)             │
│  不过 → 打回重做                         │
└─────────────────┬───────────────────────┘
                  │ 通过
┌─────────────────▼───────────────────────┐
│  Gate 2: Development (开发门禁)         │
│  标准: 测试覆盖率 ≥ 80%                  │
│  权重: 代码质量(15%) + 测试(15%)         │
│  不过 → 打回重做                         │
└─────────────────┬───────────────────────┘
                  │ 通过
┌─────────────────▼───────────────────────┐
│  Gate 3: Production (发布门禁)          │
│  标准: 综合评分 ≥ 85%                    │
│  权重: 安全(15%) + 文档(10%)             │
│  不过 → 打回或ESCALATE                   │
└─────────────────────────────────────────┘
```

**每级最多3轮迭代，超过则ESCALATE。**

**加权评分算法：**
```typescript
interface GateScore {
  requirements: number;  // 25%
  architecture: number;  // 20%
  codeQuality: number;   // 15%
  testing: number;       // 15%
  security: number;      // 15%
  documentation: number; // 10%
}

function calculateTotalScore(scores: GateScore): number {
  return (
    scores.requirements * 0.25 +
    scores.architecture * 0.20 +
    scores.codeQuality * 0.15 +
    scores.testing * 0.15 +
    scores.security * 0.15 +
    scores.documentation * 0.10
  );
}
```

## 对"极少烧ROM，持续自我进化"的价值

### 1. 渐进式质量提升
```
❌ 错误模式：最后一次性验证，失败全部重来

✅ 门禁模式：
  规划门禁通过 → 需求明确，不会后期大改
  开发门禁通过 → 代码质量OK，技术债务可控
  发布门禁通过 → 全面达标，可安全发布
```

### 2. 早期发现问题
```
案例：需求理解错误
  规划门禁发现 → 成本 = 几小时重分析
  发布门禁发现 → 成本 = 几天重实现
```

### 3. 资源分配优化
```
简单案例 → 快速通过所有门禁
复杂案例 → 在Gate 2卡3轮 → 自动ESCALATE
```

## 适用场景

| 场景 | 门禁设计 |
|------|----------|
| Adapter生成 | L1: 编译通过 → L2: 单元测试 → L3: E2E验证 |
| 规则发现 | L1: 模式提取 → L2: 验证通过 → L3: 无冲突 |
| 代码重构 | L1: 编译通过 → L2: 回归测试 → L3: 性能达标 |
| 文档生成 | L1: 格式检查 → L2: 链接有效 → L3: 人工审核 |

## 实施建议

### 最小实现（Day 1）
```bash
#!/bin/bash
# three-gate.sh

CODE_FILE="$1"
MAX_ROUNDS=3

# Gate 1: 编译
for round in $(seq 1 $MAX_ROUNDS); do
  echo "[Gate 1 - Round $round] Compiling..."
  npx tsc --noEmit "$CODE_FILE" && { echo "✓ Gate 1 passed"; break; }
  [ $round -eq $MAX_ROUNDS ] && { echo "✗ Gate 1 failed after $MAX_ROUNDS rounds"; exit 1; }
done

# Gate 2: 单元测试
for round in $(seq 1 $MAX_ROUNDS); do
  echo "[Gate 2 - Round $round] Testing..."
  npx jest "$CODE_FILE" && { echo "✓ Gate 2 passed"; break; }
  [ $round -eq $MAX_ROUNDS ] && { echo "✗ Gate 2 failed after $MAX_ROUNDS rounds"; exit 1; }
done

# Gate 3: E2E验证
for round in $(seq 1 $MAX_ROUNDS); do
  echo "[Gate 3 - Round $round] E2E..."
  node e2e-test.js "$CODE_FILE" && { echo "✓ Gate 3 passed"; break; }
  [ $round -eq $MAX_ROUNDS ] && { echo "✗ Gate 3 failed after $MAX_ROUNDS rounds"; exit 1; }
done

echo "✓ All gates passed"
```

### 进阶实现（Week 1）
```typescript
interface GateConfig {
  name: string;
  checks: CheckConfig[];
  threshold: number;
  maxRetries: number;
  onFail: 'retry' | 'escalate' | 'skip';
}

interface CheckConfig {
  name: string;
  weight: number;
  check: () => Promise<number>;  // 返回0-100分
}

class GateSystem {
  private gates: GateConfig[];

  async evaluate(artifact: Artifact): Promise<GateResult> {
    for (const gate of this.gates) {
      console.log(`\n[${gate.name}] Starting...`);

      for (let round = 1; round <= gate.maxRetries; round++) {
        const score = await this.runChecks(gate.checks, artifact);
        console.log(`  Round ${round}: ${score.toFixed(1)}% (threshold: ${gate.threshold}%)`);

        if (score >= gate.threshold) {
          console.log(`  ✓ ${gate.name} passed`);
          break;
        }

        if (round === gate.maxRetries) {
          if (gate.onFail === 'escalate') {
            return { passed: false, gate: gate.name, score, action: 'ESCALATE' };
          }
          return { passed: false, gate: gate.name, score, action: 'FAILED' };
        }

        // 自动修复重试
        artifact = await this.autoFix(artifact, gate.checks);
      }
    }

    return { passed: true, action: 'APPROVED' };
  }

  private async runChecks(checks: CheckConfig[], artifact: Artifact): Promise<number> {
    let totalScore = 0;
    let totalWeight = 0;

    for (const check of checks) {
      const score = await check.check();
      totalScore += score * check.weight;
      totalWeight += check.weight;
    }

    return totalScore / totalWeight;
  }
}

// 使用
const gateSystem = new GateSystem([
  {
    name: 'Gate 1: Compile',
    checks: [
      { name: 'syntax', weight: 0.5, check: checkSyntax },
      { name: 'types', weight: 0.5, check: checkTypes }
    ],
    threshold: 100,
    maxRetries: 3,
    onFail: 'escalate'
  },
  {
    name: 'Gate 2: Test',
    checks: [
      { name: 'unit', weight: 0.6, check: runUnitTests },
      { name: 'integration', weight: 0.4, check: runIntegrationTests }
    ],
    threshold: 80,
    maxRetries: 5,
    onFail: 'escalate'
  }
]);
```

## 参考代码片段

```typescript
// From 17.Matrix/17.0.TheOracle/evolution/evolution_selfheal.md
// 质量门禁 + 打回重做模式
/*
spec-developer 输出代码
  ↓
spec-validator 打分：78%（低于 85% 阈值）
  ↓ 打回，附上失败原因：
  "测试覆盖率 62%（要求 >80%），缺少 auth 模块的边界测试"
  ↓
spec-developer 重做（带着反馈）
  ↓
spec-validator 打分：91%（通过）

最多 3 轮，超了就停。
*/

// 加权评分示例
const weights = {
  requirements: 0.25,  // 阈值 90%
  architecture: 0.20,  // 阈值 85%
  codeQuality: 0.15,   // 阈值 80%
  testing: 0.15,       // 阈值 80%
  security: 0.15,      // 阈值 90%
  documentation: 0.10  // 阈值 85%
};
```

## 关联参考
- REF-002: Guardrail函数链
- REF-011: 退出码阻断验证
