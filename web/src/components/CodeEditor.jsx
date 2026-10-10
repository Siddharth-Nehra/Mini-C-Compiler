import Editor, { loader } from "@monaco-editor/react";
import { useEffect, useRef } from "react";

loader.config({
  paths: { vs: "https://cdn.jsdelivr.net/npm/monaco-editor@0.52.2/min/vs" },
});

export default function CodeEditor({ value, onChange, onRun, onSave }) {
  const editorRef = useRef(null);
  const runRef = useRef(onRun);
  const saveRef = useRef(onSave);

  useEffect(() => {
    runRef.current = onRun;
    saveRef.current = onSave;
  }, [onRun, onSave]);

  function handleMount(editor, monaco) {
    editorRef.current = editor;
    editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.Enter, () => {
      runRef.current?.();
    });
    editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.KeyS, () => {
      saveRef.current?.();
    });
  }

  return (
    <Editor
      language="c"
      theme="vs-dark"
      value={value}
      onChange={(next) => onChange(next ?? "")}
      onMount={handleMount}
      loading={<div className="grid h-full place-items-center text-slate-500">Loading editor…</div>}
      options={{
        fontSize: 14,
        fontFamily: "JetBrains Mono, ui-monospace, monospace",
        minimap: { enabled: false },
        automaticLayout: true,
        tabSize: 4,
        insertSpaces: true,
        autoIndent: "full",
        formatOnType: true,
        scrollBeyondLastLine: false,
        padding: { top: 12, bottom: 12 },
        lineNumbers: "on",
        renderLineHighlight: "line",
        cursorBlinking: "smooth",
        smoothScrolling: true,
        wordWrap: "on",
      }}
    />
  );
}
