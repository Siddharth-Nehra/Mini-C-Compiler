import { Cpu, LogOut, LayoutDashboard, User, Code2 } from "lucide-react";
import { Link, NavLink } from "react-router-dom";
import { useAuth } from "../context/AuthContext";

export default function TopNav({ compact }) {
  const { user, logout } = useAuth();

  const brand = (
    <Link to="/dashboard" className="group flex items-center gap-3 transition-transform hover:scale-[1.02]">
      <div className="relative grid h-10 w-10 place-items-center rounded-xl bg-gradient-to-br from-mint-400/20 via-teal-500/20 to-blue-500/10 border border-mint-400/30 text-mint-400 shadow-glow">
        <Cpu size={20} className="transition-transform duration-300 group-hover:rotate-12" />
        <span className="absolute -top-1 -right-1 flex h-2.5 w-2.5">
          <span className="animate-ping absolute inline-flex h-full w-full rounded-full bg-mint-400 opacity-75" />
          <span className="relative inline-flex rounded-full h-2.5 w-2.5 bg-mint-400" />
        </span>
      </div>
      <div>
        <span className="text-lg font-bold tracking-tight bg-gradient-to-r from-white via-slate-100 to-slate-400 bg-clip-text text-transparent">
          Mini-C Studio
        </span>
        <span className="hidden sm:block text-[10px] font-mono tracking-widest uppercase text-mint-400/80">
          C11 IDE Engine
        </span>
      </div>
    </Link>
  );

  if (compact) {
    return brand;
  }

  return (
    <header className="sticky top-0 z-40 flex items-center justify-between border-b border-white/10 bg-[#07090d]/80 px-6 py-3.5 backdrop-blur-xl transition-all">
      {brand}
      
      <nav className="flex items-center gap-3 text-sm">
        <NavLink
          to="/dashboard"
          className={({ isActive }) =>
            `flex items-center gap-2 rounded-xl px-3.5 py-2 font-medium transition-all ${
              isActive
                ? "bg-mint-400/15 text-mint-300 border border-mint-400/30 shadow-glow"
                : "text-slate-400 hover:bg-white/5 hover:text-white"
            }`
          }
        >
          <LayoutDashboard size={16} />
          <span>Dashboard</span>
        </NavLink>

        <NavLink
          to="/profile"
          className={({ isActive }) =>
            `flex items-center gap-2 rounded-xl px-3.5 py-2 font-medium transition-all ${
              isActive
                ? "bg-mint-400/15 text-mint-300 border border-mint-400/30 shadow-glow"
                : "text-slate-400 hover:bg-white/5 hover:text-white"
            }`
          }
        >
          <User size={16} />
          <span>Profile</span>
        </NavLink>

        <div className="h-5 w-[1px] bg-white/10 mx-1 hidden sm:block" />

        {user && (
          <div className="hidden sm:flex items-center gap-2.5 pl-1">
            <div className="grid h-8 w-8 place-items-center rounded-full bg-gradient-to-tr from-mint-400 to-blue-500 font-bold text-ink-950 text-xs shadow-md">
              {user.name.charAt(0).toUpperCase()}
            </div>
            <span className="text-xs font-medium text-slate-300">{user.name}</span>
          </div>
        )}

        <button
          type="button"
          onClick={logout}
          className="flex items-center gap-1.5 rounded-xl border border-rose-500/20 bg-rose-950/20 px-3 py-2 text-xs font-medium text-rose-300 transition-all hover:bg-rose-900/40 hover:border-rose-500/40"
          title="Sign out"
        >
          <LogOut size={14} />
          <span className="hidden sm:inline">Logout</span>
        </button>
      </nav>
    </header>
  );
}
