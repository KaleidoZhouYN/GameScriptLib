// 实现装备页面人物判断以及保存的基本逻辑
/* 基本步骤
1. 接受截图
2. 获取英雄头像
3. 识别英雄
4. 将英雄以及装备截图放到数据库中 */

void SaveEquipment(const cv::Mat& sc, PNet hero_net, PDatabase equip)
{
	// 获取英雄头像
	cv::Rect rect = ;
	cv::Mat hero_avatar = sc.get(rect);

	// 识别英雄
	std::string hero_name = hero_net.recog(hero_avatar);

	// 放入数据库
	if (hero_name != "null")
		equip.push(hero_name, sc, true);
}

cv::Mat ShowEquipment(const std::string& hero_name, PDatabase equip)
{
	return equip.get(hero_name);
}

cv::Mat ShowEquipment(const cv::Mat& hero_avatar, PNet hero_net, PDatabase equip)
{
	std::string hero_name = hero_net->recog(hero_avatar);
	return equip->get(hero_name);
}

// 自动获取
void AutoEquipmentRecord(PCapturer cp, PControler cl, PNet hero_net, PDatabase equip)
{
	// 首先按照战斗力排序
	cl->sort();

	while (true)
	{
		cv::Mat sc = cp->Screenshot(); 
		SaveEquipment(sc, hero_net, equip);
		cl->next_hero(); 

		// wait for message from main wighet
		// 主界面单击停止按钮
		if (waitforbreak())
			break; 
	}
}