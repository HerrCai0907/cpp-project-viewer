import { execSync } from "node:child_process";
import { existsSync } from "node:fs";
import { argv } from "node:process";

const MainEntry = `third_party/llvm/llvm`;
const BuildFolder = `third_party/llvm/build`;
const InstallPath = `third_party/llvm-release`;
const Flags = `-DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_ASSERTIONS=on -DLLVM_ENABLE_RTTI=on`;

const Config = {
  stdio: "inherit",
};

if (!existsSync(InstallPath) || argv.includes("--force")) {
  execSync(
    `cmake -S ${MainEntry} -B ${BuildFolder} ${Flags} -DLLVM_ENABLE_PROJECTS=clang -DCMAKE_INSTALL_PREFIX=${InstallPath}`,
    Config
  );
  execSync(`cmake --build ${BuildFolder} --target install`, Config);
}
