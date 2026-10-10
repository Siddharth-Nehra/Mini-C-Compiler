export default function AnimatedBackground() {
  return (
    <div className="pointer-events-none fixed inset-0 -z-10 overflow-hidden select-none">
      {/* Deep gradient base */}
      <div className="absolute inset-0 bg-[#05070a]" />

      {/* Floating Glowing Orbs */}
      <div className="animate-float-slow animate-pulse-glow absolute -top-40 -left-40 h-[600px] w-[600px] rounded-full bg-gradient-to-br from-mint-400/25 via-teal-500/15 to-transparent blur-[120px]" />
      
      <div className="animate-float-reverse animate-pulse-glow absolute -bottom-40 -right-40 h-[700px] w-[700px] rounded-full bg-gradient-to-tl from-indigo-600/20 via-blue-500/15 to-transparent blur-[140px]" />

      <div className="animate-float-slow absolute top-1/3 left-1/2 h-[450px] w-[450px] -translate-x-1/2 rounded-full bg-gradient-to-r from-purple-600/10 via-cyan-500/10 to-transparent blur-[100px]" />

      {/* Animated Grid Lines Overlay */}
      <div className="bg-grid-pattern absolute inset-0 opacity-40" />

      {/* Radial Center Highlight */}
      <div className="absolute inset-0 bg-[radial-gradient(circle_at_50%_40%,rgba(62,224,184,0.04),transparent_60%)]" />

      {/* Vignette edge shading */}
      <div className="absolute inset-0 bg-[radial-gradient(ellipse_at_center,transparent_0%,rgba(5,7,10,0.75)_100%)]" />
    </div>
  );
}
