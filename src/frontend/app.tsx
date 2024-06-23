import React, { useState, useRef } from "react";
import InheritanceTree from "./inheritance_tree";
import ProjectSelector from "./project_selector";
import DraggableCard from "./draggable_card";
import CodeCard from "./code_card";

export default function app() {
  const [project, setProject] = useState<string | null>(null);
  const [flexibleCards, setFlexibleCards] = useState<React.JSX.Element[]>([]);
  const cnt = useRef(0);

  const onChange = (value: string) => {
    setProject(value);
  };

  const onDisplayCode = (sourceCode: string, symbolName: string) => {
    const id = `${cnt.current++}`;
    let codeCard = (
      <CodeCard
        key={id}
        sourceCode={sourceCode}
        symbolName={symbolName}
        onExit={() => {
          setFlexibleCards((cards) => cards.filter((v) => v.key != id));
        }}
      />
    );
    setFlexibleCards(flexibleCards.concat([codeCard]));
  };

  return (
    <div>
      <DraggableCard title="ProjectSelector">
        <ProjectSelector onChange={onChange}></ProjectSelector>
      </DraggableCard>
      <DraggableCard title="InheritanceTree">
        <InheritanceTree
          project={project}
          onDisplayCode={onDisplayCode}
        ></InheritanceTree>
      </DraggableCard>
      {...flexibleCards}
    </div>
  );
}
