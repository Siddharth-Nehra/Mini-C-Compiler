import { useCallback, useEffect, useMemo, useState } from "react";
import { useNavigate, useParams } from "react-router-dom";
import { Maximize2, Minimize2, Play, Save, Square, Eraser, Sparkles, LayoutDashboard, Cpu, CheckCircle2, AlertCircle } from "lucide-react";
import CodeEditor from "../components/CodeEditor";
import OutputPanel from "../components/OutputPanel";
import ProgramSidebar from "../components/ProgramSidebar";
import TopNav from "../components/TopNav";
import AnimatedBackground from "../components/AnimatedBackground";
import { useToast } from "../context/ToastContext";
import {
  compileSource,
  createProgram,
  deleteProgram,
  fetchHealth,
  getProgram,
  listPrograms,
  updateProgram,
} from "../api/services";
import { STARTER_SOURCE, countSource } from "../lib/utils";

function tabText(tab, result) {
  if (!result) return "";
  if (tab === "output") return result.output || "";
  if (tab === "tokens") return result.tokens || "";
  if (tab === "ast") return result.ast || "";
  if (tab === "ir") return result.ir || "";
  if (tab === "trace") return result.trace || "";
  return (result.errors || []).map((e) => e.message).join("\n");
}

export default function Ide() {
  const { id } = useParams();
  const navigate = useNavigate();
  const { push } = useToast();
  const [programs, setPrograms] = useState([]);
  const [program, setProgram] = useState(null);
  const [source, setSource] = useState("");
  const [savedSource, setSavedSource] = useState("");
  const [running, setRunning] = useState(false);
  const [saving, setSaving] = useState(false);
  const [creating, setCreating] = useState(false);
  const [result, setResult] = useState(null);
  const [tab, setTab] = useState("output");
  const [focusMode, setFocusMode] = useState(false);
  const [compilerReady, setCompilerReady] = useState(null);
  const [loading, setLoading] = useState(true);

  const dirty = source !== savedSource;
  const counts = useMemo(() => countSource(source), [source]);

  const refreshList = useCallback(async () => {
    const items = await listPrograms();
    setPrograms(items);
    return items;
  }, []);

  useEffect(() => {
    fetchHealth()
      .then((h) => setCompilerReady(Boolean(h.compilerReady)))
      .catch(() => setCompilerReady(false));
  }, []);

  useEffect(() => {
    let cancelled = false;
    setLoading(true);
    Promise.all([getProgram(id), listPrograms()])
      .then(([item, items]) => {
        if (cancelled) return;
        setProgram(item);
        setSource(item.source);
        setSavedSource(item.source);
        setPrograms(items);
        setResult(null);
        setTab("output");
      })
      .catch((err) => {
        push(err.message, "error");
        navigate("/dashboard");
      })
      .finally(() => {
        if (!cancelled) setLoading(false);
      });
    return () => {
      cancelled = true;
    };
  }, [id, navigate, push]);

  const onSave = useCallback(async () => {
    if (!program) return;
    setSaving(true);
    try {
      const updated = await updateProgram(program.id, { source });
      setProgram(updated);
      setSavedSource(updated.source);
      await refreshList();
      push("Program saved successfully", "success");
    } catch (err) {
      push(err.message, "error");
    } finally {
      setSaving(false);
    }
  }, [program, source, push, refreshList]);

  const onRun = useCallback(async () => {
    setRunning(true);
    try {
      const data = await compileSource(source);
      setResult(data);
      if (!data.ok && data.errors?.length) {
        setTab("errors");
        push("Compilation failed", "error");
      } else if (data.ok) {
        setTab("output");
        push("Execution completed on VM", "success");
      } else {
        setTab("errors");
      }
    } catch (err) {
      setResult({
        ok: false,
        output: "",
        tokens: "",
        ast: "",
        ir: "",
        trace: "",
        errors: [{ stage: "error", message: err.message }],
      });
      setTab("errors");
      push(err.message, "error");
    } finally {
      setRunning(false);
    }
  }, [source, push]);

  useEffect(() => {
    function onKey(e) {
      if ((e.metaKey || e.ctrlKey) && e.key === "Enter") {
        e.preventDefault();
        onRun();
      }
      if ((e.metaKey || e.ctrlKey) && (e.key === "s" || e.key === "S")) {
        e.preventDefault();
        onSave();
      }
    }
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, [onRun, onSave]);

  async function onCreate() {
    setCreating(true);
    try {
      const created = await createProgram({
        name: `program-${programs.length + 1}.mc`,
        source: STARTER_SOURCE,
      });
      push("New program created", "success");
      navigate(`/ide/${created.id}`);
    } catch (err) {
      push(err.message, "error");
    } finally {
      setCreating(false);
    }
  }

  async function onDelete(programId) {
    if (!window.confirm("Delete this program?")) return;
    try {
      await deleteProgram(programId);
      const items = await refreshList();
      push("Program deleted", "success");
      if (programId === id) {
        if (items[0]) {
          navigate(`/ide/${items[0].id}`);
        } else {
          navigate("/dashboard");
        }
      }
    } catch (err) {
      push(err.message, "error");
    }
  }

  function copyTab() {
    const text = tabText(tab, result);
    if (!text) {
      push("Nothing to copy", "info");
      return;
    }
    navigator.clipboard.writeText(text).then(
      () => push("Copied to clipboard", "success"),
      () => push("Copy failed", "error")
    );
  }

  if (loading || !program) {
    return (
      <div className="relative grid min-h-screen place-items-center bg-[#05070a]">
        <AnimatedBackground />
        <div className="glass-panel flex flex-col items-center gap-4 rounded-3xl p-8 shadow-panel">
          <div className="relative flex h-12 w-12 items-center justify-center">
            <span className="absolute h-full w-full animate-ping rounded-full bg-mint-400/40" />
            <div className="h-10 w-10 animate-spin rounded-full border-2 border-mint-400 border-t-transparent" />
          </div>
          <p className="text-sm font-medium text-slate-300">Opening Mini-C Compiler IDE…</p>
        </div>
      </div>
    );
  }

  return (
    <div className="relative flex h-screen flex-col overflow-hidden bg-[#05070a]">
      <AnimatedBackground />

      {/* Top IDE Window Control Header */}
      <header className="z-20 flex items-center justify-between border-b border-white/10 bg-[#07090d]/80 px-4 py-2.5 backdrop-blur-xl">
        <div className="flex min-w-0 items-center gap-4">
          {/* macOS window controls decoration */}
          <div className="hidden items-center gap-1.5 sm:flex">
            <span className="h-3 w-3 rounded-full bg-rose-500/80 hover:opacity-100" />
            <span className="h-3 w-3 rounded-full bg-amber-500/80 hover:opacity-100" />
            <span className="h-3 w-3 rounded-full bg-emerald-500/80 hover:opacity-100" />
          </div>

          <TopNav compact />

          <div className="h-4 w-[1px] bg-white/10 hidden sm:block" />

          <div className="min-w-0">
            <div className="flex items-center gap-2">
              <span className="truncate text-sm font-semibold text-slate-200">{program.name}</span>
              {dirty ? (
                <span className="inline-flex items-center gap-1 rounded-full bg-sun-400/10 px-2 py-0.5 text-[10px] font-medium text-sun-400 border border-sun-400/20">
                  ● unsaved
                </span>
              ) : (
                <span className="inline-flex items-center gap-1 rounded-full bg-mint-400/10 px-2 py-0.5 text-[10px] font-medium text-mint-300 border border-mint-400/20">
                  <CheckCircle2 size={10} /> saved
                </span>
              )}
            </div>
            <p className="hidden text-[11px] font-mono text-slate-400 md:block">
              {counts.lines} lines · {counts.chars} chars · <span className="text-slate-300">⌘/Ctrl+Enter</span> run · <span className="text-slate-300">⌘/Ctrl+S</span> save
            </p>
          </div>
        </div>

        {/* Action Controls */}
        <div className="flex items-center gap-2">
          {/* Compiler Status Badge */}
          <span
            className={`hidden items-center gap-1.5 rounded-xl border px-3 py-1 text-xs font-semibold sm:inline-flex ${
              compilerReady
                ? "border-mint-400/30 bg-mint-400/10 text-mint-300 shadow-glow"
                : compilerReady === false
                  ? "border-rose-500/30 bg-rose-950/30 text-rose-300"
                  : "border-white/10 bg-white/5 text-slate-400"
            }`}
          >
            <span className={`h-2 w-2 rounded-full ${compilerReady ? "bg-mint-400 animate-pulse" : "bg-slate-500"}`} />
            {compilerReady ? "Compiler Ready" : compilerReady === false ? "Compiler Offline" : "Checking…"}
          </span>

          {result?.executionTimeMs != null && (
            <span className="hidden rounded-xl border border-white/10 bg-white/5 px-2.5 py-1 font-mono text-xs text-slate-300 lg:inline">
              {result.executionTimeMs} ms
            </span>
          )}

          <button
            type="button"
            onClick={() => navigate("/dashboard")}
            className="hidden items-center gap-1.5 rounded-xl border border-white/10 bg-white/5 px-3 py-1.5 text-xs font-medium text-slate-300 hover:bg-white/10 hover:text-white md:inline-flex transition-all"
          >
            <LayoutDashboard size={14} /> Dashboard
          </button>

          <button
            type="button"
            onClick={() => setSource("")}
            className="inline-flex items-center gap-1.5 rounded-xl border border-white/10 bg-white/5 px-3 py-1.5 text-xs font-medium text-slate-300 hover:bg-white/10 hover:text-white transition-all"
            title="Clear code editor"
          >
            <Eraser size={14} /> Clear
          </button>

          <button
            type="button"
            onClick={onSave}
            disabled={saving || !dirty}
            className="inline-flex items-center gap-1.5 rounded-xl border border-white/10 bg-white/10 px-3.5 py-1.5 text-xs font-medium text-slate-100 hover:bg-white/20 disabled:opacity-40 transition-all"
          >
            <Save size={14} /> {saving ? "Saving…" : "Save"}
          </button>

          <button
            type="button"
            onClick={onRun}
            disabled={running}
            className="group relative inline-flex items-center gap-1.5 overflow-hidden rounded-xl bg-gradient-to-r from-mint-400 to-teal-400 px-4 py-1.5 text-xs font-bold text-ink-950 shadow-glow transition-all hover:scale-105 hover:from-mint-300 hover:to-teal-300 disabled:opacity-60"
          >
            {running ? <Square size={14} className="animate-spin" /> : <Play size={14} fill="currentColor" />}
            <span>{running ? "Compiling…" : "Run"}</span>
          </button>

          <button
            type="button"
            onClick={() => setFocusMode((v) => !v)}
            className="rounded-xl border border-white/10 bg-white/5 p-1.5 text-slate-300 hover:bg-white/10 hover:text-white transition-all"
            title={focusMode ? "Exit focus mode" : "Full-screen focus mode"}
          >
            {focusMode ? <Minimize2 size={16} /> : <Maximize2 size={16} />}
          </button>
        </div>
      </header>

      {/* Main IDE Workspace */}
      <div className="flex min-h-0 flex-1 flex-col md:flex-row">
        {!focusMode && (
          <ProgramSidebar
            programs={programs}
            currentId={id}
            creating={creating}
            onSelect={(nextId) => navigate(`/ide/${nextId}`)}
            onCreate={onCreate}
            onDelete={onDelete}
          />
        )}
        
        <div className="flex min-h-0 min-w-0 flex-1 flex-col">
          <div className="min-h-0 flex-1 relative">
            <CodeEditor value={source} onChange={setSource} onRun={onRun} onSave={onSave} />
          </div>

          {!focusMode && (
            <OutputPanel
              tab={tab}
              setTab={setTab}
              result={result}
              running={running}
              onCopy={copyTab}
              onClear={() => {
                setResult(null);
                setTab("output");
              }}
            />
          )}
        </div>
      </div>
    </div>
  );
}
