import React, { useState } from "react";
import { Tree, TreeDataNode } from "antd";

type P = { project: string | null };
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

const Deps: React.FC<P> = (prop) => {
  const [status, setStatus] = useState<S>({
    treeData: [],
    project: null,
  });

  async function update() {
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
  }
  update();

  if (prop.project == null) {
    return <div></div>;
  }
  return (
    <div>
      <Tree showLine={true} showIcon={false} treeData={status.treeData} />
    </div>
  );
};

export default Deps;
