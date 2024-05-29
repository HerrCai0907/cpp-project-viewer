import React, { useState } from "react";
import Deps from "./deps";
import ProjectSelector from "./project_selector";
import DraggableCard from "./draggable_card";

export default function app() {
  const [project, setProject] = useState<string | null>(null);

  const onChange = (value: string) => {
    setProject(value);
  };

  return (
    <div>
      <DraggableCard title="ProjectSelector">
        <ProjectSelector onChange={onChange}></ProjectSelector>
      </DraggableCard>
      <DraggableCard title="InheritanceTree">
        <Deps project={project}></Deps>
      </DraggableCard>
    </div>
  );
}
