# REF-013: 候选规则人工确认模式 ⭐⭐⭐

## 来源
- **项目**: TheOracle evolution
- **源码位置**: evolution/evolution_knowledge.md
- **原始实现**: 自动发现 + 人工确认的双轨知识积累

## 核心机理

Agent发现候选规则，人类确认后才入库：

```
knowledge/
├── rules.yaml           ← 已确认的规则（人类批准）
├── candidates.yaml      ← Agent发现的候选规则（待确认）
├── failures.yaml        ← 失败记录（发现规则的原材料）
└── rejected.yaml        ← 被拒绝的规则（避免重复发现）
```

**发现流程：**
```
转换失败
  ↓
分析diff：output和expected哪里不同
  ↓
提取候选规则写入 candidates.yaml
  ↓
人类审查 candidates.yaml
  ↓
确认 → 移入 rules.yaml    拒绝 → 标记 rejected
```

**candidates.yaml 格式：**
```yaml
- id: CANDIDATE-004
  discovered_from: "case-007"
  timestamp: "2026-03-15T10:30:00Z"
  pattern:
    type: "color_format"
    android: "#RRGGBBAA"
    harmony: "#AARRGGBB"
  transform: "rgba_to_argb"
  confidence: 0.75
  verified_on: ["case-007"]
  status: "PENDING_REVIEW"  # PENDING_REVIEW / APPROVED / REJECTED
```

## 对"极少烧ROM，持续自我进化"的价值

### 1. 防止幻觉规则入库
```
❌ 错误：Agent发现"所有数值都要×2" → 入库 → 污染知识库

✅ 正确：
  Agent发现候选规则"数值×2"
  人类审查：只在时间戳场景适用，不是所有数值
  修改后入库：增加适用条件
```

### 2. 规则质量把关
```yaml
# 人工审查检查清单
- [ ] 模式是否清晰可描述？
- [ ] 转换逻辑是否正确？
- [ ] 适用场景是否明确？
- [ ] 与现有规则是否冲突？
- [ ] 验证案例是否足够（≥2个）？
```

### 3. 渐进式自动化
```
Phase 1 (Week 1): 100%人工审查
Phase 2 (Month 1): 置信度>0.9自动入库，其他人工审查
Phase 3 (Month 3): 仅审查冲突规则，其他自动
```

## 适用场景

| 场景 | 确认策略 |
|------|----------|
| 新规则发现 | 必须人工确认 |
| 规则修改 | 必须人工确认 |
| 规则组合 | 自动检测冲突，冲突时人工确认 |
| 规则废弃 | 人工确认后标记DEPRECATED |

## 实施建议

### 最小实现（Day 1）
```bash
#!/bin/bash
# rule-review.sh

echo "=== Pending Candidates ==="
cat knowledge/candidates.yaml

echo ""
echo "Actions: [a]pprove [r]eject [e]dit [s]kip"
read -p "Choice: " choice

case $choice in
  a)
    cat knowledge/candidates.yaml >> knowledge/rules.yaml
    > knowledge/candidates.yaml  # 清空候选
    echo "Approved and moved to rules.yaml"
    ;;
  r)
    cat knowledge/candidates.yaml >> knowledge/rejected.yaml
    > knowledge/candidates.yaml
    echo "Rejected and recorded"
    ;;
  e)
    $EDITOR knowledge/candidates.yaml
    ;;
  s)
    echo "Skipped"
    ;;
esac
```

### 进阶实现（Week 1）
```typescript
enum RuleStatus {
  PENDING = 'PENDING',
  APPROVED = 'APPROVED',
  REJECTED = 'REJECTED',
  AUTO_APPROVED = 'AUTO_APPROVED'  // 高置信度自动
}

interface CandidateRule {
  id: string;
  discoveredFrom: string;
  pattern: Pattern;
  transform: Transform;
  confidence: number;
  verifiedOn: string[];
  status: RuleStatus;
  reviewedBy?: string;
  reviewNotes?: string;
}

class RuleReviewSystem {
  private autoApproveThreshold = 0.9;
  private minVerifiedCases = 2;

  async submitCandidate(rule: CandidateRule): Promise<void> {
    // 自动审批检查
    if (rule.confidence >= this.autoApproveThreshold &&
        rule.verifiedOn.length >= this.minVerifiedCases &&
        !await this.hasConflict(rule)) {

      rule.status = RuleStatus.AUTO_APPROVED;
      await this.approveRule(rule);
      console.log(`[Auto-Approved] ${rule.id} (confidence: ${rule.confidence})`);
      return;
    }

    // 需要人工审查
    rule.status = RuleStatus.PENDING;
    await this.saveCandidate(rule);
    console.log(`[Pending Review] ${rule.id} - requires human approval`);

    // 通知人类
    await this.notifyReviewer(rule);
  }

  async reviewRule(ruleId: string, decision: 'approve' | 'reject', reviewer: string, notes?: string): Promise<void> {
    const rule = await this.loadCandidate(ruleId);

    if (decision === 'approve') {
      rule.status = RuleStatus.APPROVED;
      rule.reviewedBy = reviewer;
      rule.reviewNotes = notes;
      await this.approveRule(rule);
    } else {
      rule.status = RuleStatus.REJECTED;
      rule.reviewedBy = reviewer;
      rule.reviewNotes = notes;
      await this.rejectRule(rule);
    }
  }

  private async hasConflict(rule: CandidateRule): Promise<boolean> {
    const existingRules = await this.loadAllRules();

    for (const existing of existingRules) {
      if (this.patternsOverlap(existing.pattern, rule.pattern)) {
        return true;
      }
    }
    return false;
  }
}
```

## 参考代码片段

```yaml
# From evolution_knowledge.md
# Step 2: 自动发现 + 人工确认（Week 1）

knowledge/
  rules.yaml              ← 已确认的规则（人类批准）
  candidates.yaml         ← Agent 发现的候选规则（待确认）
  failures.yaml           ← 失败记录（发现规则的原材料）

# 发现流程：
# 转换失败
#   ↓
# 分析 diff：output 和 expected 哪里不同
#   ↓
# 提取候选规则写入 candidates.yaml
#   ↓
# 人类审查 candidates.yaml
#   ↓
# 确认 → 移入 rules.yaml    拒绝 → 标记 rejected
```

## 关联参考
- REF-009: 规则发现与学习
- REF-004: Checkpoint + Interrupt
