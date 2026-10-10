import { Code2, Play, Sparkles } from "lucide-react";

export const C_TEMPLATES = [
  {
    id: "stdio",
    name: "C stdio.h & printf",
    desc: "#include <stdio.h>, int main(), printf with format specifier, and return 0.",
    tag: "Standard C",
    source: `#include <stdio.h>

int main() {
    int a = 10;
    int b = 20;
    printf("Sum = %d\\n", a + b);
    return 0;
}
`,
  },
  {
    id: "forloop",
    name: "For Loop & Printf",
    desc: "Standard C for-loop iteration with increment operator i++ and printf.",
    tag: "For Loop",
    source: `#include <stdio.h>

int main() {
    for (int i = 1; i <= 5; i++) {
        printf("Iteration %d\\n", i);
    }
    return 0;
}
`,
  },
  {
    id: "add",
    name: "Basic Mini-C",
    desc: "Variable declarations, arithmetic addition, and simple print statement.",
    tag: "Classic Mini-C",
    source: `int a = 10;
int b = 20;
int c;

c = a + b;

print(c);
`,
  },
  {
    id: "scope",
    name: "Block Scope & Shadowing",
    desc: "Test scope isolation and variable resolution in nested blocks.",
    tag: "Scopes",
    source: `int val = 100;

{
    int val = 200;
    print(val);
}

print(val);
`,
  },
];

export default function TemplateSelector({ onSelectTemplate, creating }) {
  return (
    <div className="mt-8">
      <div className="flex items-center gap-2 mb-4">
        <Sparkles size={18} className="text-mint-400 animate-pulse" />
        <h3 className="text-base font-semibold text-slate-200">Quick Start C Templates</h3>
      </div>

      <div className="grid gap-4 sm:grid-cols-2 lg:grid-cols-4">
        {C_TEMPLATES.map((tmpl) => (
          <button
            key={tmpl.id}
            type="button"
            disabled={creating}
            onClick={() => onSelectTemplate(tmpl)}
            className="group glass-panel-interactive flex flex-col justify-between rounded-2xl p-5 text-left transition-all hover:scale-[1.02] disabled:opacity-50"
          >
            <div>
              <div className="flex items-center justify-between gap-2 mb-2">
                <span className="inline-flex items-center gap-1 rounded-full bg-mint-400/10 px-2.5 py-0.5 text-[11px] font-medium text-mint-300 border border-mint-400/20">
                  <Code2 size={12} /> {tmpl.tag}
                </span>
                <Sparkles size={14} className="text-slate-500 opacity-0 group-hover:opacity-100 transition-opacity" />
              </div>
              <p className="font-semibold text-slate-100 group-hover:text-mint-300 transition-colors">
                {tmpl.name}
              </p>
              <p className="mt-1.5 text-xs text-slate-400 leading-relaxed">
                {tmpl.desc}
              </p>
            </div>
            
            <div className="mt-4 flex items-center gap-1 text-xs font-medium text-mint-400 group-hover:translate-x-1 transition-transform">
              <span>Use template</span>
              <Play size={12} fill="currentColor" />
            </div>
          </button>
        ))}
      </div>
    </div>
  );
}
