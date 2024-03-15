import React, { useEffect, useState } from "react";
import Graphin, { GraphinData, IUserEdge, IUserNode } from "@antv/graphin";

function createNode(name: string): IUserNode {
  return {
    id: name,
    style: {
      keyshape: { size: 10, stroke: "#FF6A00", fill: "#FF6A00", fillOpacity: 0.2, strokeOpacity: 1 },
      label: { value: name, position: "top", fill: "#6DD400" },
    },
  };
}

function createEdge(source: string, target: string): IUserEdge {
  return { source, target };
}

export default function deps() {
  const [deps, setDeps] = useState<GraphinData>({ nodes: [], edges: [] });
  useEffect(() => {
    (async () => {
      const dependencies: { base: string; derived: string }[] = await (await fetch("api/dependence_graph")).json();
      let nodes = new Set<string>();
      let edges = new Array<IUserEdge>();
      for (let dep of dependencies) {
        nodes.add(dep.base);
        nodes.add(dep.derived);
        edges.push(createEdge(dep.derived, dep.base));
      }
      setDeps({ nodes: Array.from(nodes).map((n) => createNode(n)), edges });
    })();
  }, []);
  return <Graphin data={deps} style={{ background: "#363b40" }} />;
}
