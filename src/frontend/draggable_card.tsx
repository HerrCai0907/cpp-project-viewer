import React from "react";
import { Card, ConfigProvider } from "antd";
import { Button } from "antd";
import { Rnd } from "react-rnd";

const OptionalQuitButton: React.FC<{
  onExit?: () => void;
}> = (p) => {
  if (p.onExit) {
    return (
      <Button danger size="small" onClick={p.onExit}>
        close
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
    <Rnd dragHandleClassName="cursor">
      <ConfigProvider
        theme={{
          components: {
            Card: {
              colorBgContainer: "#A6D9A9",
            },
          },
        }}
      >
        <Card
          hoverable={true}
          classNames={{ header: "cursor" }}
          title={<p>{p.title}</p>}
          extra={<OptionalQuitButton onExit={p.onExit}></OptionalQuitButton>}
          bordered={true}
        >
          {p.children}
        </Card>
      </ConfigProvider>
    </Rnd>
  );
};

export default DraggableCard;
