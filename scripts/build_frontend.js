import * as esbuild from "esbuild";
import { argv } from "node:process";

const importMapping = {
  react: "React",
  "react-dom": "ReactDOM",
  "react-dom/client": "ReactDOM",
};

/**
 * @type {esbuild.BuildOptions}
 */
const options = {
  bundle: true,
  entryPoints: ["src/frontend/index.tsx"],
  outdir: "src/www",
  loader: { ".tsx": "tsx" },
  format: "esm",
  minify: false,
  sourcemap: false,
  metafile: true,
  logLevel: "info",
  treeShaking: true,
  plugins: [],
};

if (argv.includes("--watch")) {
  (await esbuild.context(options)).watch();
} else {
  await esbuild.build(options);
}
