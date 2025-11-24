import fs from "fs";
import path from "path";

export default async function handler(req, res) {
  // 数据文件存储在 Vercel serverless 的临时目录
  const filePath = path.join("/tmp", "data.json");

  if (req.method === "POST") {
    const { temp, humi, ts } = req.body;

    if (temp === undefined || humi === undefined || ts === undefined) {
      return res.status(400).json({ error: "Missing fields" });
    }

    let oldData = [];
    if (fs.existsSync(filePath)) {
      oldData = JSON.parse(fs.readFileSync(filePath, "utf8"));
    }

    oldData.push({ ts, temp, humi });
    // 保留最近 24 条数据（一天）
    if (oldData.length > 24) oldData = oldData.slice(-24);

    fs.writeFileSync(filePath, JSON.stringify(oldData, null, 2));

    return res.status(200).json({ status: "ok", saved: oldData.length });
  }

  if (req.method === "GET") {
    if (fs.existsSync(filePath)) {
      const oldData = JSON.parse(fs.readFileSync(filePath, "utf8"));
      return res.status(200).json(oldData);
    } else {
      return res.status(200).json([]);
    }
  }

  res.status(405).json({ error: "Method not allowed" });
}
