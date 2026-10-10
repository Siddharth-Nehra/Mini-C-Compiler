import { useEffect, useState } from "react";
import TopNav from "../components/TopNav";
import AnimatedBackground from "../components/AnimatedBackground";
import { useAuth } from "../context/AuthContext";
import { fetchMe } from "../api/services";
import { User, Mail, FolderCode, LogOut, ShieldCheck } from "lucide-react";

export default function Profile() {
  const { user, logout } = useAuth();
  const [count, setCount] = useState(null);

  useEffect(() => {
    fetchMe()
      .then((data) => setCount(data.programCount))
      .catch(() => setCount(0));
  }, []);

  return (
    <div className="relative min-h-screen bg-[#05070a]">
      <AnimatedBackground />
      <TopNav />

      <main className="mx-auto max-w-xl px-4 py-12">
        <div className="glass-panel overflow-hidden rounded-3xl border border-white/15 p-8 shadow-panel">
          <div className="flex items-center gap-4 border-b border-white/10 pb-6">
            <div className="grid h-14 w-14 place-items-center rounded-2xl bg-gradient-to-tr from-mint-400 to-blue-500 text-xl font-extrabold text-ink-950 shadow-glow">
              {user?.name?.charAt(0).toUpperCase()}
            </div>
            <div>
              <div className="flex items-center gap-2">
                <h1 className="text-2xl font-bold text-slate-100">{user?.name}</h1>
                <ShieldCheck size={18} className="text-mint-400" />
              </div>
              <p className="text-xs text-slate-400">Mini-C Studio Workspace Member</p>
            </div>
          </div>

          <div className="mt-6 space-y-4 text-sm">
            <div className="flex items-center gap-3 rounded-2xl bg-white/5 p-4 border border-white/10">
              <User size={18} className="text-mint-400" />
              <div>
                <p className="text-xs font-medium text-slate-400">Account Name</p>
                <p className="text-base font-semibold text-slate-100">{user?.name}</p>
              </div>
            </div>

            <div className="flex items-center gap-3 rounded-2xl bg-white/5 p-4 border border-white/10">
              <Mail size={18} className="text-cyan-400" />
              <div>
                <p className="text-xs font-medium text-slate-400">Email Address</p>
                <p className="text-base font-semibold text-slate-100">{user?.email}</p>
              </div>
            </div>

            <div className="flex items-center gap-3 rounded-2xl bg-white/5 p-4 border border-white/10">
              <FolderCode size={18} className="text-emerald-400" />
              <div>
                <p className="text-xs font-medium text-slate-400">Total Saved Programs</p>
                <p className="text-base font-semibold text-slate-100">{count ?? "…"}</p>
              </div>
            </div>
          </div>

          <button
            type="button"
            onClick={logout}
            className="mt-8 flex w-full items-center justify-center gap-2 rounded-xl border border-rose-500/30 bg-rose-950/30 py-3 text-sm font-semibold text-rose-300 hover:bg-rose-900/40 hover:border-rose-500/50 transition-all"
          >
            <LogOut size={16} /> Sign out of workspace
          </button>
        </div>
      </main>
    </div>
  );
}
