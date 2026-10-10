import { useState } from "react";
import { Link, Navigate, useNavigate } from "react-router-dom";
import { User, Mail, Lock, ArrowRight, Cpu, Sparkles } from "lucide-react";
import { useAuth } from "../context/AuthContext";
import { useToast } from "../context/ToastContext";
import AnimatedBackground from "../components/AnimatedBackground";

export default function Register() {
  const { user, register } = useAuth();
  const { push } = useToast();
  const navigate = useNavigate();
  const [name, setName] = useState("");
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
      await register(name, email, password);
      push("Workspace account created successfully", "success");
      navigate("/dashboard");
    } catch (err) {
      setError(err.message);
    } finally {
      setBusy(false);
    }
  }

  return (
    <div className="relative grid min-h-screen place-items-center bg-[#05070a] px-6 py-12 overflow-hidden">
      <AnimatedBackground />

      <form
        onSubmit={onSubmit}
        className="glass-panel w-full max-w-md rounded-3xl p-8 shadow-panel border border-white/15 relative z-10"
      >
        <div className="flex items-center gap-3 mb-6">
          <div className="grid h-10 w-10 place-items-center rounded-2xl bg-gradient-to-br from-mint-400/20 via-teal-500/20 to-blue-500/10 border border-mint-400/30 text-mint-400 shadow-glow">
            <Cpu size={20} />
          </div>
          <div>
            <h1 className="text-xl font-bold tracking-tight">Create your account</h1>
            <p className="text-xs text-slate-400">Access Mini-C online compiler IDE</p>
          </div>
        </div>

        {error && (
          <div className="mb-6 flex items-center gap-2 rounded-2xl border border-rose-500/30 bg-rose-950/40 p-3.5 text-xs text-rose-300">
            <span>{error}</span>
          </div>
        )}

        <div className="space-y-4">
          <div>
            <label className="block text-xs font-medium text-slate-300 mb-1.5">
              Full Name
            </label>
            <div className="relative">
              <User size={16} className="absolute left-3.5 top-1/2 -translate-y-1/2 text-slate-400" />
              <input
                required
                minLength={2}
                placeholder="Alice Developer"
                value={name}
                onChange={(e) => setName(e.target.value)}
                className="glass-input w-full rounded-xl pl-10 pr-4 py-3 text-sm text-slate-100 outline-none"
              />
            </div>
          </div>

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
                minLength={6}
                placeholder="At least 6 characters"
                value={password}
                onChange={(e) => setPassword(e.target.value)}
                className="glass-input w-full rounded-xl pl-10 pr-4 py-3 text-sm text-slate-100 outline-none"
              />
            </div>
          </div>
        </div>

        <button
          type="submit"
          disabled={busy}
          className="group relative mt-6 flex w-full items-center justify-center gap-2 overflow-hidden rounded-xl bg-gradient-to-r from-mint-400 to-teal-400 py-3.5 font-semibold text-ink-950 shadow-glow transition-all hover:scale-[1.02] hover:from-mint-300 hover:to-teal-300 disabled:opacity-60"
        >
          <span>{busy ? "Creating account…" : "Register Workspace"}</span>
          <ArrowRight size={16} className="transition-transform group-hover:translate-x-1" />
        </button>

        <p className="mt-6 text-center text-xs text-slate-400">
          Already have an account?{" "}
          <Link to="/login" className="font-semibold text-mint-400 hover:underline">
            Log in
          </Link>
        </p>
      </form>
    </div>
  );
}
