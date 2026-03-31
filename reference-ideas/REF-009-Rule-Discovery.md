# REF-009: 规则发现与学习模式 ⭐⭐⭐

## 来源
- **项目**: TheOracle (17.Matrix/17.0.TheOracle)
- **源码位置**: engine/knowledge/discovered-rules.yaml, 15.5-E2ETest/verify-agent.ts
- **原始实现**: 错误diff驱动的规则自动发现

## 核心机理

从失败案例的diff中自动发现转换规则：

```
案例 101 第一次运行：
  android_output: [1, 2, 3, 4, 5]   ← Ground Truth
  harmony_output: [0, 1, 2, 3, 4]   ← 原始输出
  diff: "每个元素差 1"               ← 差异分析
  → 发现规则 R01: "数组索引偏移 +1"   ← 规则提取

案例 101 第二次运行（应用 R01）：
  adapter_output: [1, 2, 3, 4, 5]
  diff: 无
  → R01 验证通过

案例 102 运行（复用 R01）：
  自动命中 R01 → 直接通过
  → R01.confidence++
```

**规则存储结构：**
```yaml
- rule_id: "R01"
  name: "数组索引偏移"
  discovered_from_case: "101"
  pattern: "鸿蒙从0开始，安卓从1开始"
  adapter_code: "array.map(v => v + 1)"
  verified_on_cases: ["101", "102", "139", "150"]  # 置信度来源
  failed_cases: []                                  # 反例
  confidence: 1.0                                   # 计算得出
```

## 对"极少烧ROM，持续自我进化"的价值

### 1. 从零开始积累
```
Day 1: 知识库为空
  ↓
处理案例 → 失败 → 发现规则
  ↓
Day 30: 知识库有50条规则
  ↓
新案例90%命中已有规则，直接通过
```

### 2. 置信度量化
```typescript
function calculateConfidence(rule: Rule): number {
  const verifiedCount = rule.verified_on_cases.length;
  const failedCount = rule.failed_cases.length;

  if (verifiedCount + failedCount === 0) return 0;

  // Wilson score interval简化版
  const phat = verifiedCount / (verifiedCount + failedCount);
  const z = 1.96;  // 95% confidence
  const n = verifiedCount + failedCount;

  return (phat + z*z/(2*n) - z * Math.sqrt((phat*(1-phat)+z*z/(4*n))/n)) / (1+z*z/n);
}
```

### 3. 规则冲突检测
```yaml
# R01: 数组元素 +1
# R02: 数组元素 ×2

案例 123 同时命中 R01 + R02：
  顺序1: +1 然后 ×2 → [1,2,3] → [2,3,4] → [4,6,8]
  顺序2: ×2 然后 +1 → [1,2,3] → [2,4,6] → [3,5,7]
  结果不同！发现冲突。
```

## 适用场景

| 场景 | 发现方式 |
|------|----------|
| 数值偏移 | diff(expected, actual) = const → 发现偏移规则 |
| 格式转换 | 字符串模式不匹配 → 发现格式转换规则 |
| 顺序调整 | 数组元素顺序不同 → 发现重排规则 |
| 单位换算 | 数值比例关系 → 发现单位换算规则 |

## 实施建议

### 最小实现（Day 1）
```typescript
// 简单的规则发现
function discoverRule(diff: Diff[]): Rule | null {
  for (const d of diff) {
    // 数值偏移模式
    if (typeof d.expected === 'number' && typeof d.actual === 'number') {
      const offset = d.expected - d.actual;
      if (offset === 1) {
        return {
          id: 'R01',
          name: '数组索引偏移',
          transform: 'value + 1'
        };
      }
    }

    // 字符串替换模式
    if (typeof d.expected === 'string' && typeof d.actual === 'string') {
      if (d.expected.replace('data-', 'data-bs-') === d.actual) {
        return {
          id: 'R02',
          name: 'Bootstrap数据属性替换',
          transform: 's/data-/data-bs-/g'
        };
      }
    }
  }
  return null;
}
```

### 进阶实现（Week 1）
```typescript
class RuleDiscoveryEngine {
  private patterns: Pattern[] = [
    { type: 'numeric_offset', matcher: matchNumericOffset },
    { type: 'string_replace', matcher: matchStringReplace },
    { type: 'array_reorder', matcher: matchArrayReorder },
    { type: 'unit_conversion', matcher: matchUnitConversion }
  ];

  discover(diff: Diff[]): DiscoveredRule[] {
    const rules: DiscoveredRule[] = [];

    for (const pattern of this.patterns) {
      const match = pattern.matcher(diff);
      if (match) {
        rules.push({
          pattern: pattern.type,
          transform: match.transform,
          confidence: 0.5,  // 初始置信度
          sampleCases: [diff.caseId]
        });
      }
    }

    return rules;
  }

  // 验证规则
  async validateRule(rule: DiscoveredRule, testCases: string[]): Promise<void> {
    let passed = 0;
    let failed = 0;

    for (const caseId of testCases) {
      const result = await applyRule(rule, caseId);
      if (result.passed) {
        passed++;
        rule.verifiedCases.push(caseId);
      } else {
        failed++;
        rule.failedCases.push(caseId);
      }
    }

    // 更新置信度
    rule.confidence = calculateConfidence(passed, failed);
  }
}
```

## 参考代码片段

```typescript
// From 17.Matrix/17.0.TheOracle/15.5-E2ETest/verify-agent.ts
/**
 * 从验证结果中提取新规则
 */
extractRule(result: VerificationResult): string | null {
  if (result.passed) return null;

  // 分析 diff 模式并提取可能的规则
  for (const d of result.diff) {
    if (d.type === 'value_mismatch') {
      // 数值不匹配，可能有规则
      if (typeof d.expected === 'number' && typeof d.actual === 'number') {
        const diff = d.expected - d.actual;
        if (diff === 1) {
          return 'R01'; // 索引偏移
        }
        if (d.expected === d.actual * 1000) {
          return 'R05'; // 时间戳单位
        }
      }

      // 字符串不匹配
      if (typeof d.expected === 'string' && typeof d.actual === 'string') {
        if (d.expected === 'true' && d.actual === 1) return 'R06';
        if (d.expected === 'false' && d.actual === 0) return 'R06';
      }
    }
  }

  return null;
}
```

## 关联参考
- REF-013: 候选规则人工确认
- REF-012: 案例级历史追踪
