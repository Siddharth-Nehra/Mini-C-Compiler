import { spawn } from "node:child_process";
import { mkdtemp, rm, writeFile } from "node:fs/promises";
import os from "node:os";
import path from "node:path";
import { fileURLToPath } from "node:url";

const ROOT = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "../..");
const COMPILER = path.join(ROOT, "mini-c");
const SOURCE_LIMIT = 100_000;
const TIMEOUT_MS = 8000;

function runCompiler(args, cwd) {
  return new Promise((resolve) => {
    const child = spawn(COMPILER, args, {
      cwd,
      env: process.env,
    });
    let stdout = "";
    let stderr = "";
    let settled = false;

    const timer = setTimeout(() => {
      if (!settled) {
        child.kill("SIGKILL");
      }
    }, TIMEOUT_MS);

    child.stdout.on("data", (chunk) => {
      stdout += chunk.toString();
    });
    child.stderr.on("data", (chunk) => {
      stderr += chunk.toString();
    });
    child.on("error", (err) => {
      if (settled) {
        return;
      }
      settled = true;
      clearTimeout(timer);
      resolve({
        code: 1,
        stdout: "",
        stderr: err.message || "Failed to start Mini-C compiler",
      });
    });
    child.on("close", (code, signal) => {
      if (settled) {
        return;
      }
      settled = true;
      clearTimeout(timer);
      resolve({
        code: signal === "SIGKILL" ? 124 : code ?? 1,
        stdout,
        stderr: signal === "SIGKILL" ? "runtime error: execution timed out\n" + stderr : stderr,
      });
    });
  });
}

function classifyLine(line) {
  if (line.includes("preprocessor error")) {
    return "preprocessor";
  }
  if (line.includes("lexical error")) {
    return "lexical";
  }
  if (line.includes("syntax error")) {
    return "syntax";
  }
  if (line.includes("semantic error")) {
    return "semantic";
  }
  if (line.includes("runtime error")) {
    return "runtime";
  }
  return null;
}

function generateSolution(kind, line) {
  if (kind === "preprocessor") {
    if (line.includes("unsupported library")) {
      const match = line.match(/unsupported library <([^>]+)>/);
      const lib = match ? match[1] : "header";
      return `Header <${lib}> is not supported. Mini-C provides standard I/O through <stdio.h> and local files ("filename.h"). Remove <${lib}> or implement the logic using standard Mini-C features.`;
    }
    return "Check #include header path or macro definition syntax.";
  }
  if (kind === "lexical") {
    const charMatch = line.match(/unexpected character '([^']+)'/);
    if (charMatch) {
      return `Remove or replace character '${charMatch[1]}'. Mini-C only accepts valid C syntax characters (keywords, identifiers, numbers, standard operators +, -, *, /, =, comparison ops, parentheses, braces, and ';').`;
    }
    return "Fix invalid characters or unclosed comments in your Mini-C source code.";
  }
  if (kind === "syntax") {
    if (line.includes("expected ';'")) {
      return "Add a missing semicolon ';' at the end of the statement or variable declaration line.";
    }
    if (line.includes("expected identifier")) {
      return "Provide a valid variable name (e.g., 'int count = 10;'). Variable names cannot start with numbers or special characters.";
    }
    if (line.includes("expected expression")) {
      return "Ensure all operators (+, -, *, /, =) have valid expressions or numbers on both sides.";
    }
    if (line.includes("expected ')'") || line.includes("expected '('")) {
      return "Check your parentheses. Ensure every opening '(' has a matching closing ')'.";
    }
    if (line.includes("expected '}'") || line.includes("expected '{'")) {
      return "Check your block braces. Ensure every opening '{' has a matching closing '}'.";
    }
    return "Check for missing semicolons, unmatched braces/parentheses, or syntax typos.";
  }
  if (kind === "semantic") {
    const varMatch = line.match(/undeclared variable '([^']+)'/);
    if (varMatch) {
      return `Declare variable '${varMatch[1]}' with 'int ${varMatch[1]};' or 'int ${varMatch[1]} = <value>;' before using it.`;
    }
    const dupMatch = line.match(/duplicate variable '([^']+)'/);
    if (dupMatch) {
      return `Variable '${dupMatch[1]}' is already declared in this scope. Remove the duplicate 'int' keyword or use a new variable name.`;
    }
    return "Check variable declarations and block scope boundaries before referencing or assigning variables.";
  }
  if (kind === "runtime") {
    if (line.includes("division by zero")) {
      return "Division by zero detected at runtime. Ensure your divisor expression is non-zero before dividing (e.g., check denominator logic).";
    }
    if (line.includes("timed out")) {
      return "Execution timed out. Ensure your while loop condition updates the counter variable so the loop terminates.";
    }
    return "Check runtime values, loop termination criteria, and division operands.";
  }
  return "Review the line and column numbers reported in the error message to locate and fix the issue.";
}

function collectErrors(...stderrs) {
  const seen = new Set();
  const errors = [];
  for (const text of stderrs) {
    for (const line of (text || "").split(/\r?\n/)) {
      const kind = classifyLine(line);
      if (!kind || seen.has(line)) {
        continue;
      }
      seen.add(line);
      errors.push({
        stage: kind,
        message: line,
        solution: generateSolution(kind, line),
      });
    }
  }
  return errors;
}

export async function compileSource(source) {
  if (typeof source !== "string") {
    throw Object.assign(new Error("Source code is required"), { status: 400 });
  }
  if (source.length > SOURCE_LIMIT) {
    throw Object.assign(new Error("Source is too large (max 100,000 characters)"), { status: 400 });
  }

  const started = Date.now();
  const dir = await mkdtemp(path.join(os.tmpdir(), "minic-"));
  const file = path.join(dir, "program.mc");

  try {
    await writeFile(file, source, "utf8");
    const [run, tokens, ast, ir, trace] = await Promise.all([
      runCompiler(["program.mc"], dir),
      runCompiler(["--tokens", "program.mc"], dir),
      runCompiler(["--ast", "program.mc"], dir),
      runCompiler(["--ir", "program.mc"], dir),
      runCompiler(["--trace", "program.mc"], dir),
    ]);

    const errors = collectErrors(run.stderr, tokens.stderr, ast.stderr, ir.stderr, trace.stderr);

    return {
      ok: run.code === 0,
      exitCode: run.code,
      output: run.stdout,
      tokens: tokens.stdout,
      ast: ast.code === 0 ? ast.stdout : "",
      ir: ir.code === 0 ? ir.stdout : "",
      trace: trace.stderr,
      errors,
      executionTimeMs: Date.now() - started,
      stages: {
        tokens: tokens.code === 0 || Boolean(tokens.stdout),
        ast: ast.code === 0,
        ir: ir.code === 0,
        run: run.code === 0,
      },
    };
  } finally {
    await rm(dir, { recursive: true, force: true });
  }
}

export function compilerPath() {
  return COMPILER;
}
