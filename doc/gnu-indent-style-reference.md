# GNU Indent Style Configuration Reference

This document analyzes the GNU Indent configuration used in `maint/code-cleanup.bash` and explains each styling option.

## Current GNU Indent Configuration

The script uses GNU Indent 2.2.11 with the following options:

### Base Style: Kernighan & Ritchie (K&R) Expansion
The configuration starts with K&R style options and then applies overrides.

### Blank Lines
- `--no-blank-lines-after-declarations` - Do not force blank lines after variable declarations
- `--no-blank-lines-after-commas` - Do not force newlines after commas in declarations  
- `--blank-lines-after-procedures` - Force blank lines after function/procedure bodies
- `--leave-optional-blank-lines` - Preserve existing blank lines in the input

### Brace Placement
- `--braces-on-if-line` - Put opening braces on same line as if/while/for statements
- `--braces-on-struct-decl-line` - Put opening braces on same line as struct declarations
- `--braces-after-func-def-line` - Put opening braces on line after function definitions
- `--cuddle-else` - Put else on same line as preceding closing brace `} else`
- `--cuddle-do-while` - Put while on same line as closing brace of do-while `} while`
- `--brace-indent0` - Do not indent braces (0 spaces)

### Indentation
- `--indent-level4` - Use 4 spaces for each indentation level
- `--continuation-indentation4` - Use 4 spaces for continuation lines
- `--case-indentation4` - Indent case labels 4 spaces from switch
- `--parameter-indentation0` - Do not indent parameters in old-style function definitions
- `--declaration-indentation1` - Indent variable names 1 space from type
- `--no-tabs` - Use spaces instead of tabs

### Comments
- `--declaration-comment-column33` - Put comments after declarations in column 33
- `--no-comment-delimiters-on-blank-lines` - Do not put comment delimiters on blank lines
- `--line-comments-indentation0` - Set indentation of line comments not to right of code to 0
- `--dont-format-first-column-comments` - Do not format comments that start in column 1
- `--dont-format-comments` - Do not format any comments
- `--comment-indentation1` - Put comments to right of code in column 1 (overrides K&R default)
- `--start-left-side-of-comments` - Put '*' character at left of comments
- `--else-endif-column1` - Put comments after #else and #endif in column 1

### Line Length and Breaking
- `--line-length100` - Set maximum line length to 100 characters
- `--honour-newlines` - Prefer to break long lines at position of newlines in input
- `--continue-at-parentheses` - Line up continued lines at parentheses
- `--break-after-boolean-operator` - Break long lines after boolean operators (&&, ||)

### Spacing
- `--space-after-cast` - Put space after cast operators: `(int) x`
- `--no-space-after-function-call-names` - No space between function name and opening parenthesis
- `--no-space-after-parentheses` - No space after '(' and before ')'
- `--space-after-for` - Put space after `for` keyword: `for (`
- `--space-after-if` - Put space after `if` keyword: `if (`  
- `--space-after-while` - Put space after `while` keyword: `while (`
- `--dont-space-special-semicolon` - Do not force space before semicolon in for loops

### Function and Procedure Formatting
- `--dont-break-procedure-type` - Keep return type on same line as function name

### Post-processing with sed
The script also applies additional formatting with sed:
- `'s/ *$//g'` - Remove trailing whitespace
- `'s/( */(/g'` - Remove space after opening parenthesis
- `'s/ *)/)/g'` - Remove space before closing parenthesis  
- `'s/if(/if (/g'` - Ensure space after 'if'
- `'s/while(/while (/g'` - Ensure space after 'while'
- `'s/do{/do {/g'` - Ensure space before opening brace in do-while
- `'s/}while/} while/g'` - Ensure space before 'while' in do-while

## Summary of Style Requirements

1. **Indentation**: 4 spaces, no tabs
2. **Braces**: K&R style (on same line for control structures, new line for functions)
3. **Line length**: 100 characters maximum
4. **Spacing**: Consistent spacing around keywords, no space in function calls
5. **Comments**: Preserve formatting, align at specific columns
6. **Blank lines**: After procedures only, preserve optional blank lines
7. **Case statements**: Indented 4 spaces from switch
8. **Boolean operators**: Break after operators on long lines