local _weatherManager = {}

local Table = require('orm.class.table')
local _weatherTable = Table("weather")

local weatherDict = {
}
weatherDict["10"] = "暴雨"
weatherDict["11"] = "大暴雨"
weatherDict["13"] = "阵雪"
weatherDict["14"] = "小雪"
weatherDict["15"] = "中雪"
weatherDict["16"] = "大雪"
weatherDict["17"] = "暴雪"
weatherDict["18"] = "雾"
weatherDict["19"] = "冻雨"
weatherDict["20"] = "沙尘暴"
weatherDict["21"] = "小到中雨"
weatherDict["22"] = "中到大雨" 
weatherDict["23"] = "大到暴雨" 
weatherDict["24"] = "暴雨到大暴雨" 
weatherDict["25"] = "大暴雨到特大暴雨" 
weatherDict["26"] = "小到中雪"
weatherDict["27"] = "中到大雪" 
weatherDict["28"] = "大到暴雪" 
weatherDict["29"] = "浮尘"
weatherDict["30"] = "扬沙" 
weatherDict["31"] = "强沙尘暴" 
weatherDict["53"] = "霾"
weatherDict["99"] = ""
weatherDict["00"] = "晴" 
weatherDict["01"] = "多云" 
weatherDict["02"] = "阴"
weatherDict["03"] = "阵雨" 
weatherDict["04"] = "雷阵雨" 
weatherDict["05"] = "雷阵雨伴有冰雹" 
weatherDict["06"] = "雨夹雪"
weatherDict["07"] = "小雨"
weatherDict["08"] = "中雨" 
weatherDict["09"] = "大雨"

local fxDict = {}
fxDict["0"] = "无持续风向"
fxDict["1"] = "东北风"
fxDict["2"] = "东风"
fxDict["3"] = "东南风" 
fxDict["4"] = "南风"
fxDict["5"] = "西南风" 
fxDict["6"] = "西风"
fxDict["7"] = "西北风" 
fxDict["8"] = "北风"
fxDict["9"] = "旋转风"

local flDict = {}
flDict["0"] = "微风"
flDict["1"] = "3-4级" 
flDict["2"] = "4-5级" 
flDict["3"] = "5-6级" 
flDict["4"] = "6-7级" 
flDict["5"] = "7-8级" 
flDict["6"] = "8-9级" 
flDict["7"] = "9-10级" 
flDict["8"] = "10-11级" 
flDict["9"] = "11-12级"

_weatherManager.getWeather = function ()
	return _weatherTable.get:all():getPureData()
end

_weatherManager.parseWeathers = function (responseStr,callback)
	local t = cjson.decode(responseStr)
	local weatherTable = _weatherTable
	local ret = {}
	if t and t.f and t.f.f1 then
		weatherTable.get:delete()
		local city = t.c.c2
		for i,v in ipairs(t.f.f1) do
			local t = {}
			t.wind = flDict[v.fg]
			t.wind_direction = fxDict[v.ff]
			t.sun_info = v.fi
			t.low = tonumber(v.fd)
			t.high = tonumber(v.fc)
			t.id = i
			t.city = city
			local weather = weatherTable(t)
			weather:save()
			table.insert(ret,weather:getPureData())
		end
	end
	if callback then
		callback(ret)
	end
end

_weatherManager.loadWeather = function (callback)
	lua_http.request({ url  = "http://mobile.weather.com.cn/data/forecast/101010100.html?_=1381891660081",
		onResponse = function (response)
			if response.http_code ~= 200 then
				if callback then
					callback(nil)
				end
			else
				lua_thread.postToThread(BusinessThreadLOGIC,"WeatherManager","parseWeathers",response.response,function(data)
					if callback then
						callback(data)
					end
				end)
			end
		end})
end

return _weatherManager
