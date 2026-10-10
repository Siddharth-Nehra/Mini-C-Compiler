import { Copy, Eraser, Check, Terminal, FileCode, Layers, Cpu, AlertTriangle, Play, Sparkles, Lightbulb, HelpCircle } from "lucide-react";
import { useState } from "react";

const TABS = [
  { id: "output", label: "Program Output", icon: Terminal },
  { id: "tokens", label: "Tokens", icon: FileCode },
  { id: "ast", label: "AST", icon: Layers },
  { id: "ir", label: "Three-Address Code", icon: Cpu },
  { id: "trace", label: "VM Trace", icon: Play },
  { id: "errors", label: "Errors & Solutions", icon: AlertTriangle },
];

function getSolutionForLine(stage, message) {
  const line = message || "";
  if (stage === "lexical") {
    const charMatch = line.match(/unexpected character '([^']+)'/);
    if (charMatch) {
      return `Remove or replace character '${charMatch[1]}'. Mini-C only accepts valid C syntax characters (keywords, identifiers, numbers, operators +, -, *, /, =, comparisons, parentheses, braces, and ';').`;
    }
    return "Fix invalid characters or unclosed comments in your Mini-C source code.";
  }
  if (stage === "syntax") {
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
  if (stage === "semantic") {
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
  if (stage === "runtime") {
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

function Empty({ text }) {
  return (
    <div className="flex h-full flex-col items-center justify-center p-6 text-center text-slate-500">
      <Sparkles size={24} className="mb-2 text-slate-600 opacity-60" />
      <p className="text-sm font-medium">{text}</p>
    </div>
  );
}

export default function OutputPanel({ tab, setTab, result, running, onCopy, onClear }) {
  const [copied, setCopied] = useState(false);
  const errorCount = result?.errors?.length || 0;

  function handleCopy() {
    onCopy();
    setCopied(true);
    setTimeout(() => setCopied(false), 2000);
  }

  let body = <Empty text="Click Run or press Ctrl+Enter to execute code and view compiler output." />;
  if (running) {
    body = (
      <div className="flex h-full items-center justify-center gap-3 text-slate-300">
        <span className="h-5 w-5 animate-spin rounded-full border-2 border-mint-400 border-t-transparent" />
        <span className="text-sm font-medium animate-pulse">Running Mini-C compiler passes…</span>
      </div>
    );
  } else if (result) {
    if (tab === "output") {
      body = result.output ? (
        <pre className="whitespace-pre-wrap font-mono text-sm leading-relaxed text-emerald-300 bg-emerald-950/20 p-4 rounded-xl border border-emerald-500/20">
          {result.output}
        </pre>
      ) : (
        <Empty text={result.ok ? "Program finished cleanly with no printed output." : "No program output. Check the Errors & Solutions tab."} />
      );
    } else if (tab === "tokens") {
      body = result.tokens ? (
        <pre className="whitespace-pre font-mono text-xs leading-relaxed text-cyan-300 bg-cyan-950/20 p-4 rounded-xl border border-cyan-500/20">
          {result.tokens}
        </pre>
      ) : (
        <Empty text="Tokens unavailable. Resolve lexical scanning issues first." />
      );
    } else if (tab === "ast") {
      body = result.ast ? (
        <pre className="whitespace-pre font-mono text-xs leading-relaxed text-purple-300 bg-purple-950/20 p-4 rounded-xl border border-purple-500/20">
          {result.ast}
        </pre>
      ) : (
        <Empty text="AST unavailable. Parse stage failed." />
      );
    } else if (tab === "ir") {
      body = result.ir ? (
        <pre className="whitespace-pre font-mono text-xs leading-relaxed text-amber-300 bg-amber-950/20 p-4 rounded-xl border border-amber-500/20">
          {result.ir}
        </pre>
      ) : (
        <Empty text="Three-Address Code (TAC) unavailable until semantic verification passes." />
      );
    } else if (tab === "trace") {
      body = result.trace ? (
        <pre className="whitespace-pre font-mono text-xs leading-relaxed text-blue-300 bg-blue-950/20 p-4 rounded-xl border border-blue-500/20">
          {result.trace}
        </pre>
      ) : (
        <Empty text="VM trace log is empty. Program may not have executed instructions." />
      );
    } else if (tab === "errors") {
      body =
        errorCount === 0 ? (
          <div className="flex flex-col items-center justify-center p-6 text-center text-mint-400">
            <Check size={28} className="mb-2 text-mint-400" />
            <p className="text-sm font-semibold">Clean Compilation</p>
            <p className="mt-1 text-xs text-slate-400">No lexical, syntax, semantic, or runtime errors detected.</p>
          </div>
        ) : (
          <div className="space-y-4 p-1">
            {result.errors.map((err, i) => {
              const solutionText = err.solution || getSolutionForLine(err.stage, err.message);
              return (
                <div
                  key={`${err.message}-${i}`}
                  className="rounded-2xl border border-rose-500/40 bg-gradient-to-br from-rose-950/40 via-rose-950/20 to-black/40 p-4 shadow-panel"
                >
                  <div className="flex items-center gap-2 mb-2">
                    <span className="rounded-lg bg-rose-500/20 px-2.5 py-1 font-mono text-[10px] font-bold uppercase tracking-wider text-rose-300 border border-rose-500/40">
                      {err.stage} Error
                    </span>
                  </div>

                  <p className="font-mono text-xs font-semibold leading-relaxed text-rose-200">
                    {err.message}
                  </p>

                  {/* Solution Box */}
                  <div className="mt-3.5 flex items-start gap-3 rounded-xl border border-mint-400/30 bg-mint-950/30 p-3.5 backdrop-blur-sm">
                    <Lightbulb size={18} className="mt-0.5 shrink-0 text-mint-400 animate-pulse" />
                    <div>
                      <p className="text-xs font-bold uppercase tracking-wider text-mint-300 mb-1 flex items-center gap-1.5">
                        Suggested Solution & Fix
                      </p>
                      <p className="text-xs leading-relaxed text-slate-200 font-sans">
                        {solutionText}
                      </p>
                    </div>
                  </div>
                </div>
              );
            })}
          </div>
        );
    }
  }

  return (
    <section className="flex h-64 shrink-0 flex-col border-t border-white/10 bg-[#07090d]/90 backdrop-blur-xl md:h-[38%]">
      {/* Panel Header Tabs */}
      <div className="flex items-center justify-between border-b border-white/10 px-3 bg-black/20">
        <div className="flex min-w-0 overflow-x-auto scrollbar-thin">
          {TABS.map((item) => {
            const Icon = item.icon;
            const isActive = tab === item.id;
            return (
              <button
                key={item.id}
                type="button"
                onClick={() => setTab(item.id)}
                className={`flex items-center gap-2 whitespace-nowrap px-4 py-2.5 text-xs font-medium transition-all ${
                  isActive
                    ? "tab-active text-mint-300 font-semibold"
                    : "text-slate-400 hover:text-slate-200 hover:bg-white/5"
                }`}
              >
                <Icon size={14} className={isActive ? "text-mint-400" : "text-slate-500"} />
                <span>{item.label}</span>
                {item.id === "errors" && errorCount > 0 && (
                  <span className="ml-1.5 rounded-full bg-rose-500/20 px-2 py-0.5 text-[10px] font-bold text-rose-300 border border-rose-500/30 animate-pulse">
                    {errorCount}
                  </span>
                )}
              </button>
            );
          })}
        </div>

        {/* Panel Tools */}
        <div className="flex shrink-0 items-center gap-1 pl-2">
          <button
            type="button"
            onClick={handleCopy}
            className="flex items-center gap-1 rounded-xl px-2.5 py-1.5 text-xs font-medium text-slate-400 hover:bg-white/10 hover:text-slate-200 transition-all"
            title="Copy tab content"
          >
            {copied ? <Check size={14} className="text-mint-400" /> : <Copy size={14} />}
            <span className="hidden sm:inline">{copied ? "Copied" : "Copy"}</span>
          </button>
          <button
            type="button"
            onClick={onClear}
            className="flex items-center gap-1 rounded-xl px-2.5 py-1.5 text-xs font-medium text-slate-400 hover:bg-white/10 hover:text-slate-200 transition-all"
            title="Clear output panel"
          >
            <Eraser size={14} />
            <span className="hidden sm:inline">Clear</span>
          </button>
        </div>
      </div>

      {/* Output Content */}
      <div className="min-h-0 flex-1 overflow-auto p-4 scrollbar-thin">
        {body}
      </div>
    </section>
  );
}
