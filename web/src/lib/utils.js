export const STARTER_SOURCE = `int a = 10;
int b = 20;
int c;

c = a + b;

print(c);
`;

export function formatTime(iso) {
  if (!iso) {
    return "";
  }
  const date = new Date(iso);
  return date.toLocaleString(undefined, {
    month: "short",
    day: "numeric",
    hour: "numeric",
    minute: "2-digit",
  });
}

export function relativeTime(iso) {
  const diff = Date.now() - new Date(iso).getTime();
  const mins = Math.round(diff / 60000);
  if (mins < 1) {
    return "just now";
  }
  if (mins < 60) {
    return `${mins}m ago`;
  }
  const hours = Math.round(mins / 60);
  if (hours < 24) {
    return `${hours}h ago`;
  }
  const days = Math.round(hours / 24);
  return `${days}d ago`;
}

export function countSource(source) {
  const lines = source.length === 0 ? 0 : source.split(/\n/).length;
  return { lines, chars: source.length };
}
