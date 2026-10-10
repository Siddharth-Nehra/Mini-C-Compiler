import { createContext, useCallback, useContext, useState } from "react";

const ToastContext = createContext(null);

export function ToastProvider({ children }) {
  const [toasts, setToasts] = useState([]);

  const push = useCallback((message, type = "info") => {
    const id = crypto.randomUUID();
    setToasts((prev) => [...prev, { id, message, type }]);
    setTimeout(() => {
      setToasts((prev) => prev.filter((t) => t.id !== id));
    }, 3200);
  }, []);

  return (
    <ToastContext.Provider value={{ push }}>
      {children}
      <div className="fixed bottom-5 right-5 z-50 flex w-[min(92vw,360px)] flex-col gap-2">
        {toasts.map((t) => (
          <div
            key={t.id}
            className={`rounded-xl border px-4 py-3 text-sm shadow-panel ${
              t.type === "error"
                ? "border-rose-500/40 bg-rose-950/80 text-rose-100"
                : t.type === "success"
                  ? "border-mint-400/40 bg-emerald-950/80 text-mint-300"
                  : "border-white/10 bg-ink-800/90 text-slate-100"
            }`}
          >
            {t.message}
          </div>
        ))}
      </div>
    </ToastContext.Provider>
  );
}

export function useToast() {
  const ctx = useContext(ToastContext);
  if (!ctx) {
    throw new Error("useToast must be used within ToastProvider");
  }
  return ctx;
}
