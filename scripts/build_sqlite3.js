import pkg from "fs-extra";
const { cpSync, existsSync, ensureDirSync } = pkg;
import { execSync } from "node:child_process";
import { join } from "node:path";
import { argv } from "node:process";

const Uri =
  "https://github.com/sqlite/sqlite/archive/refs/tags/version-3.46.1.tar.gz";

const TargetFolder = "third_party/sqlite3";
const TargetCFile = "third_party/sqlite3/sqlite3.c";
const TargetHFile = "third_party/sqlite3/sqlite3.h";

const Config = {
  stdio: "inherit",
};

export function build_sqlite3() {
  ensureDirSync(TargetFolder);
  if (
    !existsSync(TargetCFile) ||
    !existsSync(TargetHFile) ||
    argv.includes("--force")
  ) {
    const BuildFolder = join(TargetFolder, "build");
    ensureDirSync(BuildFolder);
    execSync(`wget ${Uri} -O ${join(TargetFolder, "sqlite.tar.gz")}`, Config);
    execSync(`tar xzf sqlite.tar.gz`, { ...Config, cwd: TargetFolder });
    execSync(`../sqlite-version-3.46.1/configure`, {
      ...Config,
      cwd: BuildFolder,
    });
    execSync(`make sqlite3.c`, { ...Config, cwd: BuildFolder });
    cpSync(join(BuildFolder, "sqlite3.c"), TargetCFile);
    cpSync(join(BuildFolder, "sqlite3.h"), TargetHFile);
  }
}
