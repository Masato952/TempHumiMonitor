import { kv } from "@vercel/kv";

export default async function handler(req, res) {
  const now = Date.now();

  // 读取已有数据
  let data = (await kv.get("th-data")) || [];

  if (req.method === "POST") {
    const { temp, humi } = req.body;

    if (temp === undefined || humi === undefined) {
      return res.status(400).json({ error: "Missing temp/humi" });
    }

    // 添加一条记录
    data.push({ t: now, temp, humi });

    // 删除超出24小时的数据
    const dayAgo = now - 24 * 60 * 60 * 1000;
    data = data.filter((d) => d.t >= dayAgo);

    // 存回数据库（持久化，不会丢失）
    await kv.set("th-data", data);

    return res.status(200).json({ ok: true });
  }

  if (req.method === "GET") {
    return res.status(200).json(data);
  }

  res.status(405).json({ error: "Method not allowed" });
}
