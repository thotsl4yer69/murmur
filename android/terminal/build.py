"""Build the native Android terminal offline using JDK 17+ and Android SDK 35."""
import argparse
import os
from pathlib import Path
import subprocess
import tempfile
from zipfile import ZipFile, ZIP_STORED


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sdk", type=Path, required=True)
    parser.add_argument("--jdk", type=Path, required=True)
    parser.add_argument("--keystore", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    src = Path(__file__).resolve().parent
    build = args.sdk / "build-tools/35.0.0"
    platform = args.sdk / "platforms/android-35/android.jar"
    java = args.jdk / "bin"
    suffix = ".exe" if os.name == "nt" else ""

    def run(command):
        subprocess.run([str(x) for x in command], check=True)

    args.out.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory() as tmp:
        work = Path(tmp)
        classes = work / "classes"
        classes.mkdir()
        run([java / ("javac"+suffix), "--release", "8", "-encoding", "UTF-8", "-classpath",
             platform, "-d", classes, src / "Terminal.java"])
        run([java / ("java"+suffix), "-cp", build / "lib/d8.jar", "com.android.tools.r8.D8",
             "--min-api", "26", "--lib", platform, "--output", work, *classes.rglob("*.class")])
        raw = work / "raw.apk"
        run([build / ("aapt"+suffix), "package", "-f", "-M", src / "AndroidManifest.xml",
             "-I", platform, "-F", raw])
        with ZipFile(raw, "a") as apk:
            apk.write(work / "classes.dex", "classes.dex", compress_type=ZIP_STORED)
        aligned = work / "aligned.apk"
        run([build / ("zipalign"+suffix), "-f", "4", raw, aligned])
        signer = [java / ("java"+suffix), "-jar", build / "lib/apksigner.jar"]
        run(signer + ["sign", "--ks", args.keystore, "--ks-pass", "env:MURMUR_KS_PASS",
                      "--out", args.out, aligned])
        run(signer + ["verify", "--verbose", "--print-certs", args.out])


if __name__ == "__main__":
    main()
