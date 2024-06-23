import React from "react";
import { Card, Divider, Flex } from "antd";
import Draggable from "react-draggable";
import { Button } from "antd";

const OptionalQuitButton: React.FC<{
  onExit?: () => void;
}> = (p) => {
  if (p.onExit) {
    return (
      <Button type="primary" danger size="small" onClick={p.onExit}>
        x
      </Button>
    );
  } else {
    return <></>;
  }
};

const DraggableCard: React.FC<{
  children: React.ReactNode;
  title: string;
  onExit?: () => void;
}> = (p) => {
  return (
    <Draggable handle="strong">
      <Card hoverable={true} style={{ width: 300 }}>
        <Flex justify="space-around" align="center">
          <strong className="cursor">
            <Card.Meta title={p.title}></Card.Meta>
          </strong>
          <OptionalQuitButton onExit={p.onExit}></OptionalQuitButton>
        </Flex>
        <Divider></Divider>
        {p.children}
      </Card>
    </Draggable>
  );
};

export default DraggableCard;
