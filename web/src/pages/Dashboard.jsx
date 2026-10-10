import { useEffect, useState, useMemo } from "react";
import { FileCode2, Plus, Pencil, Trash2, Clock3, Search, Cpu, Sparkles, FolderCode, Terminal, Zap } from "lucide-react";
import { useNavigate } from "react-router-dom";
import TopNav from "../components/TopNav";
import AnimatedBackground from "../components/AnimatedBackground";
import TemplateSelector from "../components/TemplateSelector";
import { useAuth } from "../context/AuthContext";
import { useToast } from "../context/ToastContext";
import { createProgram, deleteProgram, listPrograms, updateProgram, fetchHealth } from "../api/services";
import { STARTER_SOURCE, formatTime, relativeTime } from "../lib/utils";

export default function Dashboard() {
  const { user } = useAuth();
  const { push } = useToast();
  const navigate = useNavigate();
  const [programs, setPrograms] = useState([]);
  const [loading, setLoading] = useState(true);
  const [busy, setBusy] = useState(false);
  const [renameId, setRenameId] = useState(null);
  const [renameValue, setRenameValue] = useState("");
  const [searchQuery, setSearchQuery] = useState("");
  const [compilerStatus, setCompilerStatus] = useState(null);

  async function refresh() {
    const items = await listPrograms();
    setPrograms(items);
  }

  useEffect(() => {
    Promise.all([
      refresh(),
      fetchHealth().catch(() => ({ compilerReady: false }))
    ])
      .then(([_, health]) => setCompilerStatus(health?.compilerReady))
      .catch((err) => push(err.message, "error"))
      .finally(() => setLoading(false));
  }, []);

  async function onCreate(nameOverride = null, sourceOverride = null) {
    setBusy(true);
    try {
      const name = nameOverride || `program-${programs.length + 1}.mc`;
      const source = sourceOverride || STARTER_SOURCE;
      const program = await createProgram({ name, source });
      push("Program created successfully", "success");
      navigate(`/ide/${program.id}`);
    } catch (err) {
      push(err.message, "error");
    } finally {
      setBusy(false);
    }
  }

  async function onDelete(id, name) {
    if (!window.confirm(`Delete "${name}"?`)) {
      return;
    }
    try {
      await deleteProgram(id);
      setPrograms((prev) => prev.filter((p) => p.id !== id));
      push("Program deleted", "success");
    } catch (err) {
      push(err.message, "error");
    }
  }

  async function onRename(id) {
    if (!renameValue.trim()) {
      return;
    }
    try {
      const program = await updateProgram(id, { name: renameValue });
      setPrograms((prev) => prev.map((p) => (p.id === id ? program : p)));
      setRenameId(null);
      push("Renamed successfully", "success");
    } catch (err) {
      push(err.message, "error");
    }
  }

  const filteredPrograms = useMemo(() => {
    if (!searchQuery.trim()) return programs;
    const q = searchQuery.toLowerCase();
    return programs.filter((p) => p.name.toLowerCase().includes(q));
  }, [programs, searchQuery]);

  const recent = programs.slice(0, 3);

  return (
    <div className="relative min-h-screen pb-16">
      <AnimatedBackground />
      <TopNav />

      <main className="mx-auto max-w-6xl px-4 py-8 sm:px-6">
        {/* Hero Section */}
        <section className="relative overflow-hidden rounded-3xl border border-white/10 bg-gradient-to-b from-ink-900/80 via-ink-900/50 to-ink-950/80 p-8 shadow-panel backdrop-blur-xl sm:p-10">
          <div className="absolute -top-24 -right-24 h-72 w-72 rounded-full bg-mint-400/10 blur-3xl pointer-events-none" />
          <div className="absolute -bottom-24 -left-24 h-72 w-72 rounded-full bg-blue-500/10 blur-3xl pointer-events-none" />

          <div className="relative z-10 flex flex-col md:flex-row md:items-center md:justify-between gap-6">
            <div>
              <div className="inline-flex items-center gap-2 rounded-full border border-mint-400/30 bg-mint-400/10 px-3 py-1 text-xs font-semibold uppercase tracking-wider text-mint-300">
                <Sparkles size={14} className="animate-spin-slow" />
                <span>Mini-C Compiler Workspace</span>
              </div>
              <h1 className="mt-3 text-3xl font-bold tracking-tight sm:text-4xl">
                Welcome back,{" "}
                <span className="bg-gradient-to-r from-mint-300 via-teal-300 to-blue-400 bg-clip-text text-transparent">
                  {user?.name}
                </span>
              </h1>
              <p className="mt-2 max-w-2xl text-sm leading-relaxed text-slate-400 sm:text-base">
                Create, edit, compile, and analyze Mini-C source code. High-performance C11 execution pipeline with live VM bytecode, AST, and Three-Address Code output.
              </p>
            </div>

            <div className="shrink-0">
              <button
                type="button"
                onClick={() => onCreate()}
                disabled={busy}
                className="group relative inline-flex items-center gap-2 overflow-hidden rounded-2xl bg-gradient-to-r from-mint-400 to-teal-400 px-6 py-3.5 font-semibold text-ink-950 shadow-glow transition-all hover:scale-105 hover:from-mint-300 hover:to-teal-300 disabled:opacity-60"
              >
                <Plus size={20} className="transition-transform duration-300 group-hover:rotate-90" />
                <span>Create New Program</span>
              </button>
            </div>
          </div>

          {/* Stats Bar */}
          <div className="mt-8 grid grid-cols-2 gap-4 border-t border-white/10 pt-6 sm:grid-cols-4">
            <div className="flex items-center gap-3">
              <div className="grid h-10 w-10 place-items-center rounded-xl bg-white/5 border border-white/10 text-mint-400">
                <FolderCode size={18} />
              </div>
              <div>
                <p className="text-xl font-bold">{programs.length}</p>
                <p className="text-xs text-slate-400">Saved Programs</p>
              </div>
            </div>

            <div className="flex items-center gap-3">
              <div className="grid h-10 w-10 place-items-center rounded-xl bg-white/5 border border-white/10 text-cyan-400">
                <Cpu size={18} />
              </div>
              <div>
                <p className="text-xl font-bold">5 Passes</p>
                <p className="text-xs text-slate-400">Compiler Pipeline</p>
              </div>
            </div>

            <div className="flex items-center gap-3">
              <div className="grid h-10 w-10 place-items-center rounded-xl bg-white/5 border border-white/10 text-emerald-400">
                <Zap size={18} />
              </div>
              <div>
                <p className="text-xl font-bold">VM Register</p>
                <p className="text-xs text-slate-400">Execution Engine</p>
              </div>
            </div>

            <div className="flex items-center gap-3">
              <div className="grid h-10 w-10 place-items-center rounded-xl bg-white/5 border border-white/10 text-amber-400">
                <Terminal size={18} />
              </div>
              <div>
                <p className="text-xl font-bold">
                  {compilerStatus ? (
                    <span className="text-mint-400">Online</span>
                  ) : compilerStatus === false ? (
                    <span className="text-rose-400">Offline</span>
                  ) : (
                    "Checking…"
                  )}
                </p>
                <p className="text-xs text-slate-400">Backend Status</p>
              </div>
            </div>
          </div>
        </section>

        {/* Templates Selector */}
        <TemplateSelector
          creating={busy}
          onSelectTemplate={(tmpl) => onCreate(`${tmpl.id}.mc`, tmpl.source)}
        />

        {/* Recent Programs Section */}
        {recent.length > 0 && (
          <section className="mt-10">
            <div className="mb-4 flex items-center justify-between">
              <div className="flex items-center gap-2 text-slate-200">
                <Clock3 size={18} className="text-mint-400" />
                <h2 className="text-lg font-semibold">Recent Activity</h2>
              </div>
            </div>

            <div className="grid gap-4 md:grid-cols-3">
              {recent.map((p) => (
                <div
                  key={p.id}
                  onClick={() => navigate(`/ide/${p.id}`)}
                  className="glass-panel-interactive cursor-pointer rounded-2xl p-5"
                >
                  <div className="flex items-start justify-between">
                    <div className="grid h-9 w-9 place-items-center rounded-xl bg-mint-400/10 text-mint-400">
                      <FileCode2 size={18} />
                    </div>
                    <span className="rounded-full bg-white/5 px-2.5 py-1 text-[11px] font-mono text-slate-400 border border-white/10">
                      {relativeTime(p.updatedAt)}
                    </span>
                  </div>

                  <h3 className="mt-4 font-semibold text-slate-100 group-hover:text-mint-300 transition-colors">
                    {p.name}
                  </h3>
                  <p className="mt-1 font-mono text-xs text-slate-400 line-clamp-2">
                    {p.source ? p.source.trim().split("\n").slice(0, 2).join(" ; ") : "Empty source file"}
                  </p>
                </div>
              ))}
            </div>
          </section>
        )}

        {/* Saved Programs List */}
        <section className="mt-10">
          <div className="mb-4 flex flex-col gap-3 sm:flex-row sm:items-center sm:justify-between">
            <div className="flex items-center gap-2 text-slate-200">
              <FolderCode size={18} className="text-mint-400" />
              <h2 className="text-lg font-semibold">All Programs ({filteredPrograms.length})</h2>
            </div>

            <div className="relative w-full sm:w-64">
              <Search size={16} className="absolute left-3 top-1/2 -translate-y-1/2 text-slate-400" />
              <input
                type="text"
                placeholder="Search programs…"
                value={searchQuery}
                onChange={(e) => setSearchQuery(e.target.value)}
                className="w-full rounded-xl border border-white/10 bg-ink-950/70 pl-9 pr-3 py-2 text-sm text-slate-200 outline-none focus:border-mint-400/40 focus:ring-1 focus:ring-mint-400/30"
              />
            </div>
          </div>

          {loading ? (
            <div className="glass-panel grid h-40 place-items-center rounded-2xl text-slate-400">
              <div className="flex items-center gap-3">
                <span className="h-5 w-5 animate-spin rounded-full border-2 border-mint-400 border-t-transparent" />
                <span>Loading your workspace programs…</span>
              </div>
            </div>
          ) : filteredPrograms.length === 0 ? (
            <div className="glass-panel flex flex-col items-center justify-center rounded-2xl p-12 text-center">
              <div className="grid h-14 w-14 place-items-center rounded-2xl bg-white/5 border border-white/10 text-slate-500 mb-4">
                <FileCode2 size={28} />
              </div>
              <h3 className="text-lg font-semibold text-slate-200">No programs found</h3>
              <p className="mt-1 max-w-sm text-sm text-slate-400">
                {searchQuery ? "No programs match your search term." : "Create your first Mini-C program or pick a template above to get started."}
              </p>
              {!searchQuery && (
                <button
                  type="button"
                  onClick={() => onCreate()}
                  className="mt-5 inline-flex items-center gap-2 rounded-xl bg-mint-400 px-4 py-2 text-sm font-semibold text-ink-950 hover:bg-mint-300"
                >
                  <Plus size={16} /> Create Program
                </button>
              )}
            </div>
          ) : (
            <div className="overflow-hidden rounded-2xl border border-white/10 glass-panel">
              {filteredPrograms.map((p, idx) => (
                <div
                  key={p.id}
                  className={`flex flex-col gap-3 p-4 transition-colors hover:bg-white/[0.03] sm:flex-row sm:items-center sm:justify-between ${
                    idx !== filteredPrograms.length - 1 ? "border-b border-white/10" : ""
                  }`}
                >
                  <div
                    onClick={() => navigate(`/ide/${p.id}`)}
                    className="flex cursor-pointer items-center gap-3.5 min-w-0"
                  >
                    <div className="grid h-10 w-10 shrink-0 place-items-center rounded-xl bg-mint-400/10 text-mint-400 border border-mint-400/20">
                      <FileCode2 size={18} />
                    </div>
                    <div className="min-w-0">
                      <p className="font-semibold text-slate-100 truncate hover:text-mint-300 transition-colors">
                        {p.name}
                      </p>
                      <p className="text-xs text-slate-400">
                        Modified {formatTime(p.updatedAt)} ({relativeTime(p.updatedAt)})
                      </p>
                    </div>
                  </div>

                  <div className="flex items-center gap-2">
                    {renameId === p.id ? (
                      <div className="flex items-center gap-2">
                        <input
                          autoFocus
                          type="text"
                          value={renameValue}
                          onChange={(e) => setRenameValue(e.target.value)}
                          onKeyDown={(e) => {
                            if (e.key === "Enter") onRename(p.id);
                            if (e.key === "Escape") setRenameId(null);
                          }}
                          className="rounded-lg border border-mint-400/40 bg-ink-950 px-3 py-1.5 text-sm text-slate-100 outline-none focus:ring-1 focus:ring-mint-400"
                        />
                        <button
                          type="button"
                          onClick={() => onRename(p.id)}
                          className="rounded-lg bg-mint-400 px-3 py-1.5 text-xs font-semibold text-ink-950 hover:bg-mint-300"
                        >
                          Save
                        </button>
                        <button
                          type="button"
                          onClick={() => setRenameId(null)}
                          className="rounded-lg border border-white/10 px-2.5 py-1.5 text-xs text-slate-400 hover:bg-white/5"
                        >
                          Cancel
                        </button>
                      </div>
                    ) : (
                      <>
                        <button
                          type="button"
                          onClick={() => {
                            setRenameId(p.id);
                            setRenameValue(p.name);
                          }}
                          className="inline-flex items-center gap-1.5 rounded-xl border border-white/10 bg-white/5 px-3 py-1.5 text-xs font-medium text-slate-300 hover:bg-white/10 hover:text-white transition-all"
                        >
                          <Pencil size={13} /> Rename
                        </button>
                        <button
                          type="button"
                          onClick={() => onDelete(p.id, p.name)}
                          className="inline-flex items-center gap-1.5 rounded-xl border border-rose-500/20 bg-rose-950/20 px-3 py-1.5 text-xs font-medium text-rose-300 hover:bg-rose-900/40 hover:border-rose-500/40 transition-all"
                        >
                          <Trash2 size={13} /> Delete
                        </button>
                      </>
                    )}
                  </div>
                </div>
              ))}
            </div>
          )}
        </section>
      </main>
    </div>
  );
}
