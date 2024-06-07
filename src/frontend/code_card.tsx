import React from "react";
import DraggableCard from "./draggable_card";
import { CodeBlock } from "react-code-blocks";

const CodeCard: React.FC<{ sourceCode: string; symbolName: string }> = (prop) => (
  <DraggableCard
    children={<CodeBlock text={prop.sourceCode} language={"c++"} />}
    title={`source code of "${prop.symbolName}"`}
  />
);

export default CodeCard;
