#include "reader_qimen_notes.h"
#include <stddef.h>

/* Short symbol associations reviewed for this application.
 * Original private lesson files and slides are not distributed. See docs/SOURCES.md. */
static const char *stems[] = {
    "阳木。甲为首领，遁于六仪；乙丙丁三奇护甲。",
    "阴木。藤蔓、花草、书籍、传播；柔中带刚，能伸能屈。",
    "阳火。太阳、光明、热烈、急速；也取领导、权威、展示之象。",
    "阴火。灯火、星光、细小而专注；文书、证件、钥匙、电子小物。",
    "阳土。承载、包容、转化；房屋、钱财、工资、资源与流量。",
    "阴土。田园、管道、容器；弯曲、包容、思维与策划，也取积累之象。",
    "阳金。坚硬、锋利、机械、车辆；果断、规划，也取梗阻与困难之象。",
    "阴金。小金属、首饰、颗粒；创新、变革、信息，也取错误与调整之象。",
    "阳水。江海、流动、运输、航运；变化、智慧、奔波与流通。",
    "阴水。液体、饮料、茶水；弯曲、渗透、灵活与小心思。"
};
static const char *stars[] = {
    "", "天蓬属水。智慧、膨胀、流动；也取偏财、投机之象。",
    "天芮属土。学习、学生，也取问题、病象。天禽在本盘随天芮合显。",
    "天冲属木。运动、行动、冲劲、速度；也取车辆与飞行之象。",
    "天辅属木。辅助、老师、教育、文化；取二把手与辅佐之象。",
    "天禽为中宫星。本盘采用中五寄坤、天禽随芮，不作独立外宫星。",
    "天心属金。想法、领导、管理、策划；核心人物、贵重物品与坚固之象。",
    "天柱属金。声音、口舌、争论；也取律师、公检法之象。",
    "天任属土。任劳任怨、稳重、厚道；另一面也有执拗、固执之象。",
    "天英属火。漂亮、光亮、外观、展示；与形象、传播和被看见相关。"
};
static const char *doors[] = {
    "", "休门属水。休息、温润、贵人、运输；以休养和放松取象。",
    "死门属土。静止、停滞、不愿改变；与杜门的受阻难动作区分。",
    "伤门属木。车辆、震动、运动、伤害；也取军警、索债与竞争之象。",
    "杜门属木。阻隔、封闭、隐藏、技术；取受限制、难以推动之象。",
    "中宫没有八门，不把中五视为一扇门。",
    "开门属金。事业、工作、公司、开展；用来描述工作的状态与对外活动。",
    "惊门属金。声音、口舌、律师；面试、答辩、考核与争论之象。",
    "生门属土。生机、生长、钱财、总收入；房屋、果园与新事物。",
    "景门属火。消息、文书、文章、视频；曝光、通讯、学校与景区之象。"
};
static const char *gods[] = {
    "值符取重要、名贵、有影响力之象；领导、贵人、顶级资源与一家之主。",
    "腾蛇取缠绕、纠缠、反复与梦境之象；也用于描述关系黏性。",
    "太阴取隐蔽、酝酿、思考之象；状态不同，可表现为深谋远虑或犹疑。",
    "六合取多人、聚合、合作、婚姻、缘分与联系之象。",
    "白虎取急切、刚猛与伤害之象；须结合所问主题理解。",
    "玄武取隐秘、暧昧、暗耗与不明之象；也用来观察隐藏因素。",
    "九地取低、小、慢、内敛、稳守之象；与九天的高大快相对。",
    "九天取高、大、快、开阔之象；用来描述速度、规模与高度。",
    "中宫没有独立八神。八神是象义类别，不套用五行旺衰。"
};
static const char *palaces[] = {
    "", "坎一，正北，水。取流动、奔波、起伏与中子之象。",
    "坤二，西南，土。母亲、女主人、承载、稳定；也取安于现状之象。",
    "震三，正东，木。长子、震动、动态、快速；伤门的原始宫位。",
    "巽四，东南，木。风、变化、柔顺、传播；重在随势而动。",
    "中五，中央，土。本盘中五寄坤，天禽随芮；没有独立宫支与八门。",
    "乾六，西北，金。父亲、男主人、权威、主导；与值符类比。",
    "兑七，正西，金。兑上缺，取缺口、声音、不完整与反复检查之象。",
    "艮八，东北，土。少男、子孙、停止、慢、稳；取稳中求进。",
    "离九，正南，火。光亮、耀眼、展示；盛时仍需留意变化。"
};
const char *reader_qimen_note(reader_qimen_note_kind_t kind, unsigned index) {
    const char *const *table=NULL;size_t count=0;
    switch(kind) {
        case QIMEN_NOTE_STEM:table=stems;count=sizeof(stems)/sizeof(*stems);break;
        case QIMEN_NOTE_STAR:table=stars;count=sizeof(stars)/sizeof(*stars);break;
        case QIMEN_NOTE_DOOR:table=doors;count=sizeof(doors)/sizeof(*doors);break;
        case QIMEN_NOTE_GOD:table=gods;count=sizeof(gods)/sizeof(*gods);break;
        case QIMEN_NOTE_PALACE:table=palaces;count=sizeof(palaces)/sizeof(*palaces);break;
    }
    return table && index<count ? table[index] : "";
}
