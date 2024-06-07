import React, { useState } from "react";
import { Tree, TreeDataNode } from "antd";
import CodeCard from "./code_card";

type P = { project: string | null; onDisplay: (node: React.ReactNode) => any };
type S = {
  treeData: TreeDataNode[];
  project: string | null;
};

class CppClass {
  constructor(public name: string) {}
  isFirstBase: boolean = true;
  derivedClasses: CppClass[] = [];

  toTreeNode(): TreeDataNode {
    return this._toTreeNode(this.name);
  }

  _toTreeNode(key: string): TreeDataNode {
    return {
      key,
      title: this.name,
      children: this.derivedClasses
        .sort((a, b) => a.name.localeCompare(b.name))
        .map((c) => c._toTreeNode(`${key}-${c.name}`)),
    };
  }
}

function createCppClasses(dependencies: { base: string; derived: string }[]) {
  let classMapping = new Map<string, CppClass>();
  for (const { base, derived } of dependencies) {
    let baseClass = classMapping.get(base) ?? new CppClass(base);
    classMapping.set(base, baseClass);

    let derivedClass = classMapping.get(derived) ?? new CppClass(derived);
    classMapping.set(derived, derivedClass);

    baseClass.derivedClasses.push(derivedClass);
    derivedClass.isFirstBase = false;
  }
  return Array.from(classMapping.values()).sort((a, b) => a.name.localeCompare(b.name));
}

const InheritanceTree: React.FC<P> = (prop) => {
  const [status, setStatus] = useState<S>({
    treeData: [],
    project: null,
  });

  (async function () {
    if (prop.project == status.project) {
      return;
    }
    const dependencies: { base: string; derived: string }[] = await (
      await fetch(`/api/v1/projects/${prop.project}/inheritances`)
    ).json();

    const classes = createCppClasses(dependencies);
    let treeData: TreeDataNode[] = [];
    for (let info of classes) {
      if (info.isFirstBase) {
        treeData.push(info.toTreeNode());
      }
    }

    setStatus({
      project: prop.project,
      treeData: treeData,
    });
  })();

  let onSelect = async (_: unknown, { node }: { node: TreeDataNode }) => {
    const { source_code: sourceCode, symbol_name: symbolName }: { source_code: string; symbol_name: string } = await (
      await fetch(`/api/v1/projects/${prop.project}/source_codes/${node.title}`)
    ).json();
    prop.onDisplay(<CodeCard sourceCode={sourceCode} symbolName={symbolName} />);
  };

  if (prop.project == null) {
    return <div></div>;
  }
  return (
    <div>
      <Tree showLine={true} showIcon={false} treeData={status.treeData} onSelect={onSelect} />
    </div>
  );
};

export default InheritanceTree;
