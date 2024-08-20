import React, { useState } from "react";
import { Tree, TreeDataNode } from "antd";

type P = {
  project: string | null;
  onDisplayCode: (sourceCode: string, symbolName: string) => void;
};
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

function createCppClasses(
  dependencies: Record<string, string[]>,
  classes: string[]
) {
  let classMapping = new Map<string, CppClass>();
  for (const base in dependencies) {
    let baseClass = classMapping.get(base) ?? new CppClass(base);
    classMapping.set(base, baseClass);
    for (const derived of dependencies[base]) {
      let derivedClass = classMapping.get(derived) ?? new CppClass(derived);
      classMapping.set(derived, derivedClass);
      baseClass.derivedClasses.push(derivedClass);
      derivedClass.isFirstBase = false;
    }
  }
  for (const c of classes) {
    if (!classMapping.has(c)) {
      classMapping.set(c, new CppClass(c));
    }
  }
  return Array.from(classMapping.values()).sort((a, b) => {
    return a.name.localeCompare(b.name);
  });
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
    const getInheritances = async (): Promise<Record<string, string[]>> => {
      return await (
        await fetch(`/api/v1/projects/${prop.project}/inheritances`)
      ).json();
    };
    const getClasses = async (): Promise<string[]> => {
      return await (
        await fetch(`/api/v1/projects/${prop.project}/classes`)
      ).json();
    };
    const [dependencies, classes] = await Promise.all([
      getInheritances(),
      getClasses(),
    ]);

    const cppClasses = createCppClasses(dependencies, classes);
    let treeData: TreeDataNode[] = [];
    for (let cppClass of cppClasses) {
      if (cppClass.isFirstBase) {
        treeData.push(cppClass.toTreeNode());
      }
    }

    setStatus({
      project: prop.project,
      treeData: treeData,
    });
  })();

  let onSelect = async (_: unknown, { node }: { node: TreeDataNode }) => {
    const {
      source_code: sourceCode,
      symbol_name: symbolName,
    }: { source_code: string; symbol_name: string } = await (
      await fetch(`/api/v1/projects/${prop.project}/source_codes/${node.title}`)
    ).json();
    prop.onDisplayCode(sourceCode, symbolName);
  };

  if (prop.project == null) {
    return <div></div>;
  }
  return (
    <div>
      <Tree
        showLine={true}
        showIcon={false}
        treeData={status.treeData}
        onSelect={onSelect}
      />
    </div>
  );
};

export default InheritanceTree;
