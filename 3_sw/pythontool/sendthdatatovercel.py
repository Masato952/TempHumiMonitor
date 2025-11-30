import requests
import time
import random

# 改成你的 Vercel API URL
API_URL = "https://temp-humi-monitor.vercel.app/api/upload"

# 模拟一天的 24 条数据（每小时一条）
def generate_fake_data():
    data_list = []
    current_ts = int(time.time())  # 当前时间戳
    for i in range(24):
        ts = current_ts - (23 - i) * 3600  # 过去24小时
        temp = round(random.uniform(20, 30), 1)  # 20~30℃随机温度
        humi = round(random.uniform(50, 70), 1)  # 50~70%随机湿度
        data_list.append({"ts": ts, "temp": temp, "humi": humi})
    return data_list

def send_data(data_list):
    for entry in data_list:
        response = requests.post(API_URL, json=entry)
        if response.status_code == 200:
            print(f"已发送: {entry}")
        else:
            print(f"发送失败: {entry}, 状态码: {response.status_code}")

if __name__ == "__main__":
    fake_data = generate_fake_data()
    send_data(fake_data)
