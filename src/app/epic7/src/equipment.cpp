// ʵ��װ��ҳ�������ж��Լ�����Ļ����߼�
/* ��������
1. ���ܽ�ͼ
2. ��ȡӢ��ͷ��
3. ʶ��Ӣ��
4. ��Ӣ���Լ�װ����ͼ�ŵ����ݿ��� */

void SaveEquipment(const cv::Mat& sc, PNet hero_net, PDatabase equip)
{
	// ��ȡӢ��ͷ��
	cv::Rect rect = ;
	cv::Mat hero_avatar = sc.get(rect);

	// ʶ��Ӣ��
	std::string hero_name = hero_net.recog(hero_avatar);

	// �������ݿ�
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

// �Զ���ȡ
void AutoEquipmentRecord(PCapturer cp, PControler cl, PNet hero_net, PDatabase equip)
{
	// ���Ȱ���ս��������
	cl->sort();

	while (true)
	{
		cv::Mat sc = cp->Screenshot(); 
		SaveEquipment(sc, hero_net, equip);
		cl->next_hero(); 

		// wait for message from main wighet
		// �����浥��ֹͣ��ť
		if (waitforbreak())
			break; 
	}
}