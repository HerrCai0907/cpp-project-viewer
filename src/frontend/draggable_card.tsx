import React from "react";
import { Card, Divider } from "antd";
import Draggable from "react-draggable";

const { Meta } = Card;

const draggable_card: React.FC<{ children: React.ReactNode; title: string }> = ({ children, title }) => {
  return (
    <Draggable handle="strong">
      <Card hoverable={true}>
        <strong className="cursor">
          <Meta title={title}></Meta>
        </strong>
        <Divider></Divider>
        {children}
      </Card>
    </Draggable>
  );
};

export default draggable_card;
