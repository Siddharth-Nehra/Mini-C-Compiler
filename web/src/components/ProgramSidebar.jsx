import { FileCode2, Plus, Trash2 } from "lucide-react";
import { relativeTime } from "../lib/utils";

export default function ProgramSidebar({
  programs,
  currentId,
  onSelect,
  onCreate,
  onDelete,
  creating,
}) {
  return (
    <aside className="flex w-full shrink-0 flex-col border-b border-white/10 bg-ink-900/80 md:h-full md:w-64 md:border-b-0 md:border-r">
      <div className="flex items-center justify-between px-3 py-3">
        <p className="text-xs font-semibold uppercase tracking-[0.16em] text-slate-400">Programs</p>
        <button
          type="button"
          onClick={onCreate}
          disabled={creating}
          className="inline-flex items-center gap-1 rounded-lg bg-mint-400/15 px-2 py-1 text-xs text-mint-300 hover:bg-mint-400/25 disabled:opacity-50"
        >
          <Plus size={14} /> New
        </button>
      </div>
      <div className="min-h-0 flex-1 overflow-auto scrollbar-thin px-2 pb-3">
        {programs.length === 0 ? (
          <div className="rounded-xl border border-dashed border-white/10 px-3 py-8 text-center text-sm text-slate-500">
            No saved programs yet.
          </div>
        ) : (
          programs.map((p) => (
            <div
              key={p.id}
              className={`group mb-1 flex items-center rounded-xl ${
                p.id === currentId ? "bg-mint-400/10 text-white" : "text-slate-300 hover:bg-white/5"
              }`}
            >
              <button type="button" onClick={() => onSelect(p.id)} className="min-w-0 flex-1 px-3 py-2.5 text-left">
                <span className="flex items-center gap-2">
                  <FileCode2 size={15} className="shrink-0 text-mint-400" />
                  <span className="truncate text-sm font-medium">{p.name}</span>
                </span>
                <span className="mt-0.5 block pl-6 text-[11px] text-slate-500">{relativeTime(p.updatedAt)}</span>
              </button>
              <button
                type="button"
                title="Delete"
                onClick={() => onDelete(p.id)}
                className="mr-1 rounded-lg p-1.5 text-slate-500 opacity-0 hover:bg-rose-950/50 hover:text-rose-200 group-hover:opacity-100"
              >
                <Trash2 size={14} />
              </button>
            </div>
          ))
        )}
      </div>
    </aside>
  );
}
