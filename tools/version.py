# PlatformIO post-script: defines OWL_VERSION for project sources only.
# Tag vX.Y.Z on HEAD -> "X.Y.Z"; otherwise "X.Y.Z-dev.N+sha" (N commits after the last tag),
# or "0.0.0-dev+sha" before the first tag. "-dirty" is appended for uncommitted changes.
import subprocess

Import("projenv")  # noqa: F821  (provided by SCons)


def git(*args):
    try:
        return subprocess.check_output(["git", *args], stderr=subprocess.DEVNULL, text=True).strip()
    except Exception:
        return ""


def owl_version():
    sha = git("rev-parse", "--short", "HEAD") or "unknown"
    dirty = "-dirty" if git("status", "--porcelain", "--untracked-files=no") else ""
    desc = git("describe", "--tags", "--match", "v[0-9]*", "--long")
    if desc:
        tag, n, _ = desc.rsplit("-", 2)
        base = tag[1:]
        return (base if n == "0" else f"{base}-dev.{n}+{sha}") + dirty
    return f"0.0.0-dev+{sha}{dirty}"


version = owl_version()
print(f"OWL_VERSION {version}")
projenv.Append(CPPDEFINES=[("OWL_VERSION", '\\"%s\\"' % version)])  # noqa: F821
