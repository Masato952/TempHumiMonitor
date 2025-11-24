import { MongoClient } from "mongodb";

const uri = process.env.MONGODB_URI;

export default async function handler(req, res) {
  const client = new MongoClient(uri);
  try {
    await client.connect();
    const db = client.db("TempHumiDB");       // 数据库名称
    const collection = db.collection("Data"); // 集合名称
    const docs = await collection
      .find()
      .sort({ timestamp: -1 })
      .limit(50)
      .toArray();                              // 获取最近50条
    res.status(200).json(docs);
  } catch (err) {
    console.error(err);
    res.status(500).send("Database error");
  } finally {
    await client.close();
  }
}
