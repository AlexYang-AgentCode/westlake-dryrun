# REF-012: 案例级历史追踪模式 ⭐⭐⭐

## 来源
- **项目**: TheOracle, 17.0.TheOracle
- **源码位置**: engine/history/*.json
- **原始实现**: 逐案例追踪尝试历史

## 核心机理

每个案例有独立的历史文件，记录所有尝试：

```json
// engine/history/101.json
{
  "case_id": "101",
  "attempts": [
    {
      "attempt": 1,
      "verdict": "FAIL",
      "diff": [{"expected": 1, "actual": 0}],
      "discovered_rule": "R01",
      "timestamp": "2026-03-15T10:00:00Z"
    },
    {
      "attempt": 2,
      "verdict": "PASS",
      "diff": [],
      "applied_rule": "R01",
      "timestamp": "2026-03-15T10:00:05Z"
    }
  ],
  "solved": true,
  "total_attempts": 2,
  "final_rule_set": ["R01"]
}
```

**历史聚合统计：**
```json
// engine/history/batch-result.json
{
  "timestamp": "2026-03-15T12:00:00Z",
  "summary": {
    "total": 199,
    "passed": 150,
    "failed": 30,
    "pending": 19,
    "pass_rate": 0.75,
    "avg_attempts": 2.4
  },
  "by_tier": {
    "T1": { "total": 50, "passed": 48, "pass_rate": 0.96 },
    "T2": { "total": 50, "passed": 42, "pass_rate": 0.84 },
    "T3": { "total": 50, "passed": 35, "pass_rate": 0.70 },
    "T4": { "total": 49, "passed": 25, "pass_rate": 0.51 }
  }
}
```

## 对"极少烧ROM，持续自我进化"的价值

### 1. 失败模式分析
```typescript
// 分析哪些规则经常一起失败
const failurePatterns = analyzeFailureHistory();
/*
{
  "R01+R02": { fails: 5, total: 20 },  // 组合成功率75%
  "R01+R03": { fails: 15, total: 20 }, // 组合成功率25% → 发现冲突！
}
*/
```

### 2. 进展追踪
```
case_001: 2 attempts → PASS
  ↓
case_002: 1 attempt → PASS
  ↓
case_003: 5 attempts → FAIL (ESCALATED)
  ↓ 系统学习到：某些案例类型需要特殊处理
```

### 3. 知识复用
```typescript
// 新案例优先复用已验证成功的规则组合
function suggestRulesForCase(caseId: string): string[] {
  const similarCases = findSimilarCases(caseId);
  const successfulRules = similarCases
    .filter(c => c.solved)
    .flatMap(c => c.final_rule_set);

  // 按出现频率排序
  return countBy(successfulRules)
    .sort((a, b) => b.count - a.count)
    .map(r => r.rule);
}
```

## 适用场景

| 场景 | 历史记录内容 |
|------|-------------|
| Adapter生成 | 每次尝试的代码、编译结果、diff |
| 规则验证 | 应用规则、验证结果、置信度变化 |
| E2E测试 | 截图路径、SSIM分数、通过/失败 |
| 性能测试 | 执行时间、内存占用、对比基线 |

## 实施建议

### 最小实现（Day 1）
```bash
# 目录结构
mkdir -p evidence/case_{001..100}/$(date +%Y%m%d)

# 每次尝试记录结果
echo '{
  "attempt": 1,
  "verdict": "FAIL",
  "timestamp": "'$(date -Iseconds)'"
}' > evidence/case_001/$(date +%Y%m%d-%H%M%S).json
```

### 进阶实现（Week 1）
```typescript
interface AttemptRecord {
  attempt: number;
  timestamp: string;
  verdict: 'PASS' | 'FAIL' | 'PENDING';
  diff?: Diff[];
  error?: string;
  rules_applied?: string[];
  rules_discovered?: string[];
  execution_time_ms?: number;
}

interface CaseHistory {
  case_id: string;
  attempts: AttemptRecord[];
  solved: boolean;
  total_attempts: number;
  final_rules?: string[];
  escalated?: boolean;
  escalate_reason?: string;
}

class HistoryManager {
  private historyDir: string;

  async recordAttempt(caseId: string, record: AttemptRecord): Promise<void> {
    const history = await this.loadHistory(caseId);
    history.attempts.push(record);
    history.total_attempts = history.attempts.length;
    history.solved = record.verdict === 'PASS';

    await this.saveHistory(caseId, history);
  }

  async getStats(): Promise<HistoryStats> {
    const allHistory = await this.loadAllHistory();

    return {
      total: allHistory.length,
      passed: allHistory.filter(h => h.solved).length,
      failed: allHistory.filter(h => !h.solved && !h.escalated).length,
      escalated: allHistory.filter(h => h.escalated).length,
      avg_attempts: average(allHistory.map(h => h.total_attempts))
    };
  }

  // 发现失败模式
  async findFailurePatterns(): Promise<FailurePattern[]> {
    const unsolvedCases = await this.loadUnsolvedCases();

    // 聚类分析失败原因
    const patterns = groupBy(unsolvedCases, c =>
      c.attempts.slice(-1)[0]?.error?.substring(0, 50)
    );

    return Object.entries(patterns).map(([pattern, cases]) => ({
      pattern,
      count: cases.length,
      case_ids: cases.map(c => c.case_id)
    }));
  }
}
```

## 参考代码片段

```typescript
// From 17.Matrix/17.0.TheOracle/15.1-WorkFlow/workflow-agent.ts
// 基于历史选择下一个案例
getNextCase(): TestCase | null {
  const unsolvedCases: Array<{ case: TestCase; priority: number }> = [];

  for (const testCase of this.cases) {
    const history = this.loadHistory(testCase.id);

    if (!history) {
      // 未处理过的案例，高优先级
      unsolvedCases.push({ case: testCase, priority: testCase.tier * 1000 + 100 });
    } else if (!history.solved && history.attempts.length < 10) {
      // 已失败但未解决的案例，中等优先级（考虑尝试次数）
      unsolvedCases.push({
        case: testCase,
        priority: testCase.tier * 1000 - history.attempts.length
      });
    }
  }

  unsolvedCases.sort((a, b) => b.priority - a.priority);
  return unsolvedCases[0]?.case || null;
}
```

```json
// From 17.Matrix/17.0.TheOracle/engine/history/101.json
{
  "case_id": "101",
  "attempts": [
    {"attempt": 1, "verdict": "FAIL", "diff": [{"expected": 1, "actual": 0}]},
    {"attempt": 2, "verdict": "PASS", "diff": []}
  ],
  "solved": true,
  "total_attempts": 2
}
```

## 关联参考
- REF-001: Progress Ledger
- REF-009: 规则发现与学习
