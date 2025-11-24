import { useEffect, useState } from "react";

export default function Home() {
  const [data, setData] = useState([]);

  useEffect(() => {
    async function fetchData() {
      const res = await fetch("/api/fetch-temp");
      const json = await res.json();
      setData(json);
    }
    fetchData();
  }, []);

  return (
    <div style={{ padding: "20px", fontFamily: "Arial" }}>
      <h1>温湿度数据</h1>
      {data.length === 0 ? (
        <p>正在加载...</p>
      ) : (
        <ul>
          {data.map((d, idx) => (
            <li key={idx}>
              {new Date(d.timestamp).toLocaleString()} - 温度: {d.temperature}°C, 湿度: {d.humidity}%
            </li>
          ))}
        </ul>
      )}
    </div>
  );
}
