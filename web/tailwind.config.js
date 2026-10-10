/** @type {import('tailwindcss').Config} */
export default {
  content: ["./index.html", "./src/**/*.{js,jsx}"],
  theme: {
    extend: {
      fontFamily: {
        sans: ["Outfit", "ui-sans-serif", "system-ui", "sans-serif"],
        mono: ["JetBrains Mono", "ui-monospace", "SFMono-Regular", "monospace"],
      },
      colors: {
        ink: {
          950: "#07090d",
          900: "#0c1118",
          850: "#101821",
          800: "#15202c",
          700: "#1c2b3b",
          600: "#2a3d52",
        },
        mint: {
          300: "#7ef0d2",
          400: "#3ee0b8",
          500: "#1ec9a3",
        },
        sun: {
          400: "#f0c36a",
        },
      },
      boxShadow: {
        glow: "0 0 40px rgba(62, 224, 184, 0.12)",
        panel: "0 20px 60px rgba(0, 0, 0, 0.35)",
      },
    },
  },
  plugins: [],
};
