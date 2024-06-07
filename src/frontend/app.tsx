import React, { useState } from "react";
import InheritanceTree from "./inheritance_tree";
import ProjectSelector from "./project_selector";
import DraggableCard from "./draggable_card";

export default function app() {
  const [project, setProject] = useState<string | null>(null);
  const [flexibleCards, setFlexibleCards] = useState<React.ReactNode[]>([]);

  const onChange = (value: string) => {
    setProject(value);
  };

  const onDisplay = (node: React.ReactNode) => {
    setFlexibleCards(flexibleCards.concat([node]));
  };

  return (
    <div>
      <DraggableCard title="ProjectSelector">
        <ProjectSelector onChange={onChange}></ProjectSelector>
      </DraggableCard>
      <DraggableCard title="InheritanceTree">
        <InheritanceTree project={project} onDisplay={onDisplay}></InheritanceTree>
      </DraggableCard>
      {...flexibleCards}
    </div>
  );
}
