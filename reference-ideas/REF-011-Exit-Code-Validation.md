# REF-011: 退出码阻断验证模式 ⭐⭐⭐

## 来源
- **项目**: ts-hooks, 17.1 多项目
- **源码位置**: 17.Matrix/17.0.TheOracle/evolution/evolution_validation.md, evolution_hooks.md
- **原始实现**: Claude Code PreToolUse/PostToolUse Hook

## 核心机理

用exit code作为不可绕过的阻断机制：

```bash
# exit code 语义
exit 0 → 静默通过（Hook输出不显示）
exit 1 → 警告（显示但不阻断）
exit 2 → 阻断（操作被拒绝）
```

**PreToolUse Hook 示例：**
```python
#!/usr/bin/env python3
# check-commit.py
import json, sys, re

input_data = json.load(sys.stdin)
command = input_data.get("tool_input", {}).get("command", "")

# 拦截危险命令
if re.search(r"rm\s+-rf", command):
    print("Blocked: rm -rf is dangerous", file=sys.stderr)
    sys.exit(2)  # 阻断，命令不执行

# 建议替代命令
if re.search(r"^grep\b", command):
    print("Use 'rg' instead of 'grep'", file=sys.stderr)
    sys.exit(2)

sys.exit(0)  # 通过
```

**PostToolUse Hook 示例：**
```javascript
// quality-check.js
// Agent编辑.ts文件后自动触发

const result = runChecks(filePath);
if (result.hasErrors) {
  console.error(result.errors);
  process.exit(2); // 阻止文件保存
}
process.exit(0);
```

## 对"极少烧ROM，持续自我进化"的价值

### 1. 防伪强度最高
```
❌ 软性验证："请检查代码质量" → Agent可能忽略
✅ 硬性阻断：exit 2 → 操作被拒绝，必须修复
```

### 2. 自动化执行
```
Agent执行命令
  ↓ PreToolUse Hook自动触发
验证脚本运行
  ↓ exit 2
命令被拒绝，Agent看到错误信息
  ↓
Agent修复后重试
```

### 3. 可组合的多层验证
```json
{
  "hooks": {
    "PreToolUse": [
      { "command": "check-dangerous-commands.py" },  // L1
      { "command": "check-commit-readiness.sh" }     // L2
    ],
    "PostToolUse": [
      { "matcher": "Write|Edit", "command": "check-code-quality.js" },  // L3
      { "matcher": "Write|Edit", "command": "verify-gate.sh" }          // L4
    ]
  }
}
```

## 适用场景

| 场景 | Hook类型 | 验证内容 |
|------|----------|----------|
| 危险命令拦截 | PreToolUse | rm -rf, git push --force |
| 代码提交前 | PreToolUse | verify-gate通过 |
| 编辑后检查 | PostToolUse | 编译通过、ESLint |
| 会话结束前 | Stop | 所有任务完成 |

## 实施建议

### 最小实现（Day 1）
```bash
#!/bin/bash
# verify-gate.sh — 不通过就不能commit

set -euo pipefail

INPUT="$1"
OUTPUT="$2"

# 检查1：文件存在
if [ ! -f "$OUTPUT" ]; then
  echo "FAIL: 输出文件不存在" >&2
  exit 2
fi

# 检查2：不为空
if [ ! -s "$OUTPUT" ]; then
  echo "FAIL: 输出文件为空" >&2
  exit 2
fi

# 检查3：语法合法
npx tsc --noEmit "$OUTPUT" 2>/dev/null || {
  echo "FAIL: 语法错误" >&2
  exit 2
}

echo "PASS"
exit 0
```

**集成到Git Hook：**
```bash
# .git/hooks/pre-commit
#!/bin/bash
bash scripts/verify-gate.sh input/sample.yaml output/sample.json || exit 2
```

### 进阶实现（Week 1）
```typescript
// Hook配置系统
interface HookConfig {
  event: 'PreToolUse' | 'PostToolUse' | 'Stop' | 'UserPromptSubmit';
  matcher?: string;  // 工具名称匹配（如 Write|Edit）
  command: string;   // 执行脚本
  type: 'block' | 'warn' | 'notify';  // exit 2 / 1 / 0
}

const hooks: HookConfig[] = [
  {
    event: 'PreToolUse',
    matcher: 'Bash',
    command: 'scripts/check-dangerous-commands.py',
    type: 'block'
  },
  {
    event: 'PostToolUse',
    matcher: 'Write|Edit',
    command: 'scripts/check-code-quality.js',
    type: 'block'
  },
  {
    event: 'PostToolUse',
    matcher: 'Write|Edit',
    command: 'scripts/update-stats.sh',
    type: 'notify'  // 不阻断，只记录
  }
];

// Hook执行器
class HookExecutor {
  async execute(hook: HookConfig, context: any): Promise<HookResult> {
    const result = await runCommand(hook.command, context);

    if (result.exitCode === 2 && hook.type === 'block') {
      return { blocked: true, reason: result.stderr };
    }

    if (result.exitCode === 1 && hook.type === 'warn') {
      return { blocked: false, warning: result.stderr };
    }

    return { blocked: false };
  }
}
```

## 参考代码片段

```json
// From evolution_hooks.md
// .claude/settings.local.json
{
  "hooks": {
    "PreToolUse": [
      {
        "matcher": "Bash",
        "hooks": [
          {
            "type": "command",
            "command": "python3 scripts/check-commit.py"
          }
        ]
      }
    ]
  }
}
```

```python
# From evolution_hooks.md
# check-commit.py 示例
#!/usr/bin/env python3
import json, sys, re

input_data = json.load(sys.stdin)
command = input_data.get("tool_input", {}).get("command", "")

# 拦截包含 git commit 的命令
if re.search(r"git\s+commit", command):
    result = subprocess.run(["bash", "scripts/verify-gate.sh"], capture_output=True)
    if result.returncode != 0:
        print("BLOCKED: verify-gate.sh failed", file=sys.stderr)
        sys.exit(2)

sys.exit(0)
```

## 关联参考
- REF-002: Guardrail函数链
- REF-007: 角色隔离验证
