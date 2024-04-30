import React, { useEffect, useState } from "react";
import Deps from "./deps";
import ProjectSelector from "./project_selector";

export default function app() {
  const [project, setProject] = useState<string | null>(null);

  const onChange = (value: string) => {
    console.log("select ", value);
    setProject(value);
  };

  return (
    <div>
      <h2>dependence map</h2>
      <div>
        <ProjectSelector onChange={onChange}></ProjectSelector>
        <Deps project={project}></Deps>
      </div>
    </div>
  );
}
