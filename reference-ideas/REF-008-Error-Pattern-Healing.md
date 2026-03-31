# REF-008: 错误模式匹配修复模式 ⭐⭐⭐

## 来源
- **项目**: BasicToCpp (17.Matrix/17.1.BasicToCpp)
- **源码位置**: engine/self-heal/self-healing-loop.ts
- **原始实现**: 编译错误驱动的自动修复

## 核心机理

编译器报什么错 → 按模式匹配 → 应用修复 → 重试：

```typescript
// 核心循环
for (let attempt = 1; attempt <= 10; attempt++) {
  const result = compile(code);
  if (result.success) break;

  // 模式匹配修复
  for (const rule of fixRules) {
    const match = result.error.match(rule.pattern);
    if (match) {
      code = rule.fix(code, match);
      break;  // 一次只修一个问题，避免连锁反应
    }
  }
}
```

**实际Fix规则：**

| 错误模式 | 修复动作 |
|---------|----------|
| `redefinition of 'X'` | 删除重复的变量声明 |
| `'X' was not declared` | 在main()开头添加声明 |
| `label 'L' used but not defined` | 在文件末尾添加空标签 |
| `expected ';' before '}'` | 在指定位置添加分号 |

## 对"极少烧ROM，持续自我进化"的价值

### 1. 快速修复常见错误
```
Adapter生成后编译失败
  ↓
匹配错误模式（10ms）
  ↓
自动应用修复（10ms）
  ↓
重新编译

vs 人工修复：读错误 → 分析 → 修改 → 保存 → 编译（30秒+）
```

### 2. 规则可累积
```typescript
// 新错误模式 → 添加规则 → 未来自动修复
const fixRules: FixRule[] = [
  // 已有规则
  { pattern: /redefinition/, fix: fixRedefinition },
  { pattern: /not declared/, fix: fixNotDeclared },
  // 新发现的规则
  { pattern: /cannot find module '(.+)'/, fix: fixMissingImport }
];
```

### 3. 安全限制
```typescript
const SAFETY_LIMITS = {
  maxAttempts: 10,        // 最多10次尝试
  maxTimeMs: 30000,       // 最多30秒
  oneFixPerRound: true,   // 一次只修一个问题
  rollbackOnFail: true    // 失败回滚到最初版本
};
```

## 适用场景

| 场景 | 错误模式示例 |
|------|-------------|
| 编译错误 | 语法错误、类型错误、缺失导入 |
| 运行时错误 | 空指针、数组越界、类型转换失败 |
| 测试失败 | 断言失败、超时、异常抛出 |
| 格式错误 | YAML解析失败、JSON格式错误 |

## 实施建议

### 最小实现（Day 1）
```bash
#!/bin/bash
# auto-fix.sh

CODE_FILE="$1"
MAX_ATTEMPTS=5

for attempt in $(seq 1 $MAX_ATTEMPTS); do
  # 编译检查
  npx tsc --noEmit "$CODE_FILE" 2> error.log
  if [ $? -eq 0 ]; then
    echo "✓ Fixed after $attempt attempts"
    exit 0
  fi

  # 模式匹配修复
  if grep -q "redefinition of" error.log; then
    VAR=$(grep -oP "redefinition of '\K[^']+" error.log | head -1)
    echo "Fixing: redefinition of $VAR"
    # 删除重复声明（简化示例）
    sed -i "0,/let $VAR/s//\/\/ FIXED: let $VAR/" "$CODE_FILE"
  elif grep -q "was not declared" error.log; then
    VAR=$(grep -oP "'\K[^']+(?=' was not declared)" error.log | head -1)
    echo "Fixing: $VAR not declared"
    # 在文件开头添加声明
    sed -i "1s/^/let $VAR: any; \/\/ AUTO-DECLARED\n/" "$CODE_FILE"
  else
    echo "✗ Unknown error, cannot auto-fix"
    exit 1
  fi
done

echo "✗ Failed after $MAX_ATTEMPTS attempts"
exit 1
```

### 进阶实现（Week 1）
```typescript
// SelfHealingLoop 类
class SelfHealingLoop {
  private fixRules: FixRule[] = [
    {
      name: "redefinition",
      pattern: /redefinition of '(.+)'/,
      fix: (code: string, match: RegExpMatchArray): string => {
        const varName = match[1];
        const lines = code.split('\n');
        let foundFirst = false;
        return lines
          .filter(line => {
            if (line.includes(`let ${varName}`) || line.includes(`const ${varName}`)) {
              if (foundFirst) return false;  // 删除重复
              foundFirst = true;
            }
            return true;
          })
          .join('\n');
      }
    },
    {
      name: "not_declared",
      pattern: /'(.+)' was not declared/,
      fix: (code, match) => {
        const varName = match[1];
        return code.replace(
          /^(function|const|let|var|class)\s/,
          `let ${varName}: any;\n$1 `
        );
      }
    }
  ];

  async heal(code: string, errorOutput: string): Promise<string | null> {
    for (const rule of this.fixRules) {
      const match = errorOutput.match(rule.pattern);
      if (match) {
        console.log(`  [AutoFix] Applying: ${rule.name}`);
        return rule.fix(code, match);
      }
    }
    return null;  // 无匹配规则
  }
}

// 使用
const healer = new SelfHealingLoop();
for (let attempt = 1; attempt <= 10; attempt++) {
  const compileResult = await compile(code);
  if (compileResult.success) break;

  const fixed = await healer.heal(code, compileResult.error);
  if (!fixed) {
    console.log("  Cannot auto-fix, escalating...");
    break;
  }
  code = fixed;
}
```

## 参考代码片段

```typescript
// From 17.Matrix/17.1.BasicToCpp/engine/self-heal/self-healing-loop.ts
// 修复规则库
private fixRules: Array<{
  pattern: RegExp;
  fix: (code: string, error: string) => string;
  description: string;
}> = [
  {
    // 修复变量重复定义
    pattern: /redefinition of '(.+)'/,
    fix: (code, error) => {
      const match = error.match(/redefinition of '(.+)'/);
      if (match) {
        const varName = match[1];
        const lines = code.split('\n');
        let foundFirst = false;
        return lines.filter(line => {
          if (line.includes(`int ${varName}`) || line.includes(`string ${varName}`)) {
            if (foundFirst) return false;
            foundFirst = true;
          }
          return true;
        }).join('\n');
      }
      return code;
    },
    description: "Remove duplicate variable declarations"
  },
  // ... 更多规则
];
```

## 关联参考
- REF-009: 规则发现与学习
- REF-012: 案例级历史追踪
