import { useState } from "react";
import { Link, Navigate, useNavigate } from "react-router-dom";
import { Cpu, Mail, Lock, ArrowRight, Sparkles, Code2, Layers, Terminal, KeyRound } from "lucide-react";
import { useAuth } from "../context/AuthContext";
import { useToast } from "../context/ToastContext";
import AnimatedBackground from "../components/AnimatedBackground";

export default function Login() {
  const { user, login } = useAuth();
  const { push } = useToast();
  const navigate = useNavigate();
  const [email, setEmail] = useState("");
  const [password, setPassword] = useState("");
  const [busy, setBusy] = useState(false);
  const [error, setError] = useState("");

  if (user) {
    return <Navigate to="/dashboard" replace />;
  }

  async function onSubmit(e) {
    e.preventDefault();
    setBusy(true);
    setError("");
    try {
      await login(email, password);
      push("Welcome back to Mini-C Studio", "success");
      navigate("/dashboard");
    } catch (err) {
      setError(err.message);
    } finally {
      setBusy(false);
    }
  }

  function fillDemo() {
    setEmail("demo@minic.com");
    setPassword("password123");
    setError("");
  }

  return (
    <div className="relative grid min-h-screen lg:grid-cols-2 bg-[#05070a] overflow-hidden">
      <AnimatedBackground />

      {/* Left Feature Section */}
      <section className="relative hidden flex-col justify-between p-12 lg:flex border-r border-white/10 bg-ink-900/40 backdrop-blur-xl">
        <div className="flex items-center gap-3">
          <div className="grid h-11 w-11 place-items-center rounded-2xl bg-gradient-to-br from-mint-400/20 via-teal-500/20 to-blue-500/10 border border-mint-400/30 text-mint-400 shadow-glow">
            <Cpu size={22} />
          </div>
          <div>
            <p className="text-xl font-bold tracking-tight bg-gradient-to-r from-white to-slate-300 bg-clip-text text-transparent">
              Mini-C Studio
            </p>
            <p className="text-xs font-mono text-mint-400/90 tracking-wider uppercase">
              Online Compiler & Virtual Machine
            </p>
          </div>
        </div>

        <div className="max-w-lg space-y-6">
          <div className="inline-flex items-center gap-2 rounded-full border border-mint-400/30 bg-mint-400/10 px-3.5 py-1.5 text-xs font-semibold uppercase tracking-wider text-mint-300 shadow-glow">
            <Sparkles size={14} className="animate-spin-slow" />
            <span>Full 5-Pass C11 Compiler Pipeline</span>
          </div>

          <h1 className="text-4xl font-extrabold leading-tight tracking-tight sm:text-5xl">
            A real C compiler backend in your browser.
          </h1>

          <p className="text-base leading-relaxed text-slate-400">
            Write code with syntax highlighting, run programs on a stackless register Virtual Machine, and inspect exact tokens, AST, Three-Address Code (TAC), and VM traces.
          </p>

          <div className="grid grid-cols-3 gap-3 pt-4">
            <div className="glass-panel rounded-2xl p-4 text-center">
              <Code2 size={20} className="mx-auto mb-2 text-mint-400" />
              <p className="text-xs font-semibold text-slate-200">Lexer & Parser</p>
              <p className="text-[11px] text-slate-400">Tokens & AST</p>
            </div>
            <div className="glass-panel rounded-2xl p-4 text-center">
              <Layers size={20} className="mx-auto mb-2 text-cyan-400" />
              <p className="text-xs font-semibold text-slate-200">Semantics & IR</p>
              <p className="text-[11px] text-slate-400">TAC Generation</p>
            </div>
            <div className="glass-panel rounded-2xl p-4 text-center">
              <Terminal size={20} className="mx-auto mb-2 text-emerald-400" />
              <p className="text-xs font-semibold text-slate-200">Virtual Machine</p>
              <p className="text-[11px] text-slate-400">Bytecode Exec</p>
            </div>
          </div>
        </div>

        <div className="flex items-center gap-2 font-mono text-xs text-slate-500 border-t border-white/10 pt-4">
          <span className="h-2 w-2 rounded-full bg-mint-400 animate-pulse" />
          <span>Lexer → Parser → Symbol Table → TAC → VM Machine</span>
        </div>
      </section>

      {/* Right Login Form */}
      <section className="flex items-center justify-center p-6 sm:p-12">
        <form
          onSubmit={onSubmit}
          className="glass-panel w-full max-w-md rounded-3xl p-8 shadow-panel border border-white/15"
        >
          <div className="mb-6 flex items-center justify-between">
            <div>
              <h2 className="text-2xl font-bold tracking-tight">Log in to workspace</h2>
              <p className="mt-1 text-sm text-slate-400">
                Enter your account details to access saved programs and compiler IDE.
              </p>
            </div>
          </div>

          {error && (
            <div className="mb-6 flex flex-col gap-1.5 rounded-2xl border border-rose-500/30 bg-rose-950/40 p-3.5 text-xs text-rose-300">
              <span className="font-semibold">{error}</span>
            </div>
          )}

          <div className="space-y-4">
            <div>
              <label className="block text-xs font-medium text-slate-300 mb-1.5">
                Email Address
              </label>
              <div className="relative">
                <Mail size={16} className="absolute left-3.5 top-1/2 -translate-y-1/2 text-slate-400" />
                <input
                  type="email"
                  required
                  placeholder="name@example.com"
                  value={email}
                  onChange={(e) => setEmail(e.target.value)}
                  className="glass-input w-full rounded-xl pl-10 pr-4 py-3 text-sm text-slate-100 outline-none"
                />
              </div>
            </div>

            <div>
              <label className="block text-xs font-medium text-slate-300 mb-1.5">
                Password
              </label>
              <div className="relative">
                <Lock size={16} className="absolute left-3.5 top-1/2 -translate-y-1/2 text-slate-400" />
                <input
                  type="password"
                  required
                  placeholder="••••••••"
                  value={password}
                  onChange={(e) => setPassword(e.target.value)}
                  className="glass-input w-full rounded-xl pl-10 pr-4 py-3 text-sm text-slate-100 outline-none"
                />
              </div>
            </div>
          </div>

          <div className="mt-4 flex items-center justify-between">
            <button
              type="button"
              onClick={fillDemo}
              className="inline-flex items-center gap-1.5 rounded-lg border border-mint-400/30 bg-mint-400/10 px-2.5 py-1 text-xs font-medium text-mint-300 hover:bg-mint-400/20 transition-all"
            >
              <KeyRound size={13} />
              <span>Use Demo Credentials</span>
            </button>
          </div>

          <button
            type="submit"
            disabled={busy}
            className="group relative mt-5 flex w-full items-center justify-center gap-2 overflow-hidden rounded-xl bg-gradient-to-r from-mint-400 to-teal-400 py-3.5 font-semibold text-ink-950 shadow-glow transition-all hover:scale-[1.02] hover:from-mint-300 hover:to-teal-300 disabled:opacity-60"
          >
            <span>{busy ? "Authenticating…" : "Log in to Studio"}</span>
            <ArrowRight size={16} className="transition-transform group-hover:translate-x-1" />
          </button>

          <p className="mt-6 text-center text-xs text-slate-400">
            Don't have an account?{" "}
            <Link to="/register" className="font-semibold text-mint-400 hover:underline">
              Create a workspace account
            </Link>
          </p>
        </form>
      </section>
    </div>
  );
}
