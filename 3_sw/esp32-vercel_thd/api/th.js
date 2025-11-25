import { kv } from "@vercel/kv";

export default async function handler(req, res) {
  const now = Date.now();

  try {
    // 确认 KV 环境变量是否存在
    if (!process.env.KV_URL || !process.env.KV_REST_API_TOKEN) {
      return res.status(500).json({ error: "KV environment variables not set" });
    }

    // 确认方法
    if (req.method === "POST") {
      const { temp, humi } = req.body;

      if (temp === undefined || humi === undefined) {
        return res.status(400).json({ error: "Missing temp or humi in body" });
      }

      // 读取已有数据
      let data = (await kv.get("th-data")) || [];

      // 添加新记录
      data.push({ t: now, temp, humi });

      // 保留最近24小时的数据
      const dayAgo = now - 24 * 60 * 60 * 1000;
      data = data.filter(d => d.t >= dayAgo);

      // 写回 KV
      await kv.set("th-data", data);

      return res.status(200).json({ success: true });
    }

    if (req.method === "GET") {
      const data = (await kv.get("th-data")) || [];
      return res.status(200).json(data);
    }

    res.setHeader("Allow", ["GET", "POST"]);
    return res.status(405).json({ error: `Method ${req.method} not allowed` });

  } catch (err) {
    console.error("API error:", err);
    return res.status(500).json({ error: "Internal Server Error", detail: err.message });
  }
}
