import React, { useEffect, useState } from "react";
import { Select } from "antd";

type Project = {
  name: string;
};

// Filter `option.label` match the user type `input`
const filterOption = (input: string, option?: { label: string; value: string }) =>
  (option?.label ?? "").toLowerCase().includes(input.toLowerCase());

export default function project_selector(prop: { onChange: (value: string) => void }) {
  const [list, setList] = useState<Project[]>([]);
  useEffect(() => {
    (async () => {
      const projectList: Project[] = await (await fetch("/api/v1/projects")).json();
      setList(projectList);
    })();
  }, []);

  return (
    <Select
      showSearch
      placeholder="Select Project"
      optionFilterProp="children"
      onChange={prop.onChange}
      filterOption={filterOption}
      options={list.map((v) => {
        return { value: v.name, label: v.name };
      })}
    />
  );
}
