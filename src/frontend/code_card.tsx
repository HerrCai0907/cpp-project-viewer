import React from "react";
import DraggableCard from "./draggable_card";
import { CodeBlock } from "react-code-blocks";

const CodeCard: React.FC<{
  sourceCode: string;
  symbolName: string;
  onExit: () => void;
}> = (p) => (
  <DraggableCard
    children={<CodeBlock text={p.sourceCode} language={"c++"} />}
    title={`source code of "${p.symbolName}"`}
    onExit={p.onExit}
  />
);

export default CodeCard;
