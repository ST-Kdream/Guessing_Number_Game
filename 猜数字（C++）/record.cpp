#include "record.h"
#include "rank.h"
#include "Qt_gui.h"

//记录保存函数定义
bool record_save(bool& is_win, int& difficulty, int& attempts, int& max_num, int& chance,int& EP) {
	time_t now = time(0);
	char time_str[26];
	ctime_s(time_str, sizeof time_str, &now);

	std::ofstream record_file("game_record.txt", std::ios::app);
	if (record_file.is_open()) 
    {
		record_file << "游戏时间: " << time_str;
		record_file << "游戏结果：" << (is_win ? "胜利" : "失败") << "  ";
		record_file << "获得经验：" << (is_win ? EP : 0) << std::endl;
		record_file << "难度等级: " << difficulty << std::endl;
		record_file << "最大数字: " << max_num << std::endl;
		record_file << "猜测次数: " << attempts << "/" << chance << std::endl;
		record_file << "----------------------------------------" << std::endl;
        record_file.close();
        return true;
	}
	else 
    {
        return false;
	}
}

//玩家信息显示函数定义
bool player_information(std::stringstream& output) 
{
   	std::ifstream player_file("player_information.txt");
	if (player_file.is_open()) 
    {
		std::string player_line;
		output << "玩家信息记录：" << std::endl;
		while (getline(player_file, player_line)) 
        {
			output << player_line << std::endl;
		}
		player_file.close();
		return true;
	}
	else 
    {
		output << "无法打开记录文件，无法显示玩家信息。" << std::endl;
		return false;
	}
}

//初始化玩家信息函数定义
void player_init() 
{
    QString name;
    std::string name_str;
    QDialog player(nullptr);
    player.setWindowTitle("玩家注册");
    player.resize(300, 200);
    QVBoxLayout* mainlay = new QVBoxLayout(&player);

    QLineEdit* nameinput = new QLineEdit(&player);
    nameinput->setPlaceholderText("请输入昵称：");

    QDialogButtonBox* btn = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel ,&player);
    mainlay->addWidget(nameinput);
    mainlay->addWidget(btn);

    QObject::connect(btn, &QDialogButtonBox::accepted, &player, &QDialog::accept);
    QObject::connect(btn, &QDialogButtonBox::rejected, &player, &QDialog::reject);

    if (player.exec() == QDialog::Accepted)
    {
        name = nameinput->text().trimmed();
        name_str = name.toStdString();
    }
    
    if (!name_str.empty())
    {
        std::ofstream player_init("player_information.txt");
        if (player_init.is_open())
        {
            player_init << name_str << std::endl;
            player_init << "总经验值:" << 0 << std::endl;
            player_init << "段位：迷雾探索者" << std::endl;
            player_init.close();
            QMessageBox::information(nullptr, "初始化成功", QString("欢迎你，%1").arg(name));
        }
        else
        {
            QMessageBox::warning(nullptr, "初始化失败", "出现位置问题，设置失败");
        }
    }
    else
    {
        QMessageBox::warning(nullptr, "错误", "无法获取有效信息");
    }
}

//玩家经验值信息更新函数定义
bool player_update(int& update_EP,int& sum_EP) 
{
    std::string EP_line;  
    std::string first_line;  
    std::string rank_line;
    // 用binary模式打开，避免换行符转换导致的乱码
    std::fstream player_file("player_information.txt", std::ios::in | std::ios::out | std::ios::binary);  

    if (player_file.is_open()) 
    {
        // 读取第一行（玩家名）和第二行（经验值行）
        getline(player_file, first_line);
        getline(player_file, EP_line);
        getline(player_file, rank_line);

        // 查找冒号（兼容中文“：”和英文“:”）
        size_t pos = EP_line.find("：");
        if (pos == std::string::npos)
        {
            pos = EP_line.find(":");  // 补充英文冒号兼容
        }

        if (pos != std::string::npos) 
        {
            // 截取冒号后内容，并过滤出纯半角数字（解决格式错误核心）
            std::string num_str = EP_line.substr(pos + 1);  // 跳过冒号
            std::string pure_num;  // 临时变量，用于存储过滤后的数字
            // 用ASCII范围判断半角数字（替代isdigit，避免误判）
            for (char c : num_str) 
            {
                if (c >= '0' && c <= '9')    // 只保留0-9的半角数字
                {  
                    pure_num += c;
                }
            }

            // 检查是否提取到有效数字
            try
            {
                if (pure_num.empty())
                {
                    player_file.close();
                    throw std::runtime_error("更新玩家信息错误");
                }
            }
            catch (const std::exception& e)
            {
                player_file.close();
                return false;
            }
            

            // 转换数字并计算总经验值
            try 
            {
                sum_EP = stoi(pure_num) + update_EP;  
            }
            catch (const std::invalid_argument&) 
            {
                return false;
                player_file.close();
            }
            catch (const std::out_of_range&)
            {
                return false;
                player_file.close();;
            }

            // 关闭原文件，用truncate模式重新打开写入（解决truncate成员错误）
            player_file.close();
            std::ofstream out_file("player_information.txt", std::ios::out | std::ios::trunc | std::ios::binary);
            if (out_file.is_open()) 
            {
                // 写回玩家名和新经验值行（用\r\n避免换行符乱码）
                out_file << first_line << "\r\n";
                out_file << "总经验值：" << sum_EP << "\r\n";
                out_file << rank_line << "\r\n";
                out_file.close();
                return true;
            }
            else 
            {
                return false;
            }

        }
        else 
        {
            // 未找到冒号的错误处理
            player_file.close();
            return false;
        }

    }
    else 
    {
        // 文件打开失败的错误处理
        return false;
    }
}

//段位更新函数头文件
bool rank_update(std::string rank_name)
{
    std::string rank_line = "玩家段位为：" + rank_name;
    std::vector <std::string> fline;
    std::ifstream in_file("player_information.txt");
    if (in_file.is_open())
    {
        std::string line;
        while (getline(in_file, line))
        {
            fline.push_back(line);
        }
        in_file.close();
    }
    else
        return false;

    if (fline.size() == 3)
    {
        fline[2] = rank_line;
    }
    else
        return false;

    std::ofstream out_file("player_information.txt");
    if (out_file.is_open())
    {
        for (const auto& out_line : fline)
        {
            out_file << out_line << "\r\n";
        }
        out_file.close();
        return true;
    }
    else
    {
        std::cout << "玩家信息文件无法打开" << std::endl;
        QMessageBox::warning(nullptr, "错误", "玩家信息打开失败");
        return false;
    }
}

//显示游戏规则函数定义
void show_rules(std::stringstream& output) 
{
	std::ifstream rule_file("game_rules.txt");
	if (rule_file.is_open()) 
    {
		std::string rule_line;
		while (getline(rule_file, rule_line)) 
        {
			output << rule_line << std::endl;
		}
		rule_file.close();
    }
	else
    {
		output << "无法打开规则文件，无法显示游戏规则。" << std::endl;
        QMessageBox::warning(nullptr, "错误", "游戏规则文件打开失败");
	}
}

//统一更新函数定义
bool update_all(int update_EP)
{
    
    if (!player_update(update_EP, sum_EP))
    {
        QMessageBox::warning(nullptr, "错误", "玩家经验值更新失败");
        return false;
    }
    std::string rank_name = Rank::update_rank(sum_EP, rankings);
    if (!rank_update(rank_name))
    {
        QMessageBox::warning(nullptr, "错误", "玩家段位更新失败");
        return false;
    }
    QString newinfo = QString("总经验：%1，段位：%2").arg(sum_EP).arg(rank_name);
    QMessageBox::information(nullptr, "信息更新", newinfo);
    return true;
}
