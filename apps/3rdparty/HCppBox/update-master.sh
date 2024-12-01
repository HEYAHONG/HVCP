#!/bin/bash

#检查工具是否存在,$1为待检查的工具名。
function CheckTool
{
	[  -n "$1"  ] ||
	{
		echo -e  "\033[41;37mCheckTool 参数错误!!\r\n\033[40;37m";
		return 255;
	};
	ToolPath=`which $1`;
	[ -e "$ToolPath" ] ||
	{
		 echo -e "\033[41;37m$1 不存在，请先安装此工具\r\n\033[40;37m";
		 return 255;
	};
	return 0;
}

CheckTool git
[ $? -eq 0 ] || exit;
CheckTool mkdir
[ $? -eq 0 ] || exit;
CheckTool rsync
[ $? -eq 0 ] || exit;


#获取当前目录
slef_path=
# shellcheck disable=SC2128  # ignore array expansion warning
if [ -n "${BASH_SOURCE-}" ]
then
        self_path="${BASH_SOURCE}"
elif [ -n "${ZSH_VERSION-}" ]
then
        self_path="${(%):-%x}"
else
        return 1
fi
# shellcheck disable=SC2169,SC2169,SC2039  # unreachable with 'dash'
if [[ "$OSTYPE" == "darwin"* ]]; then
        # convert possibly relative path to absolute
        script_dir="$(realpath_int "${self_path}")"
        # resolve any ../ references to make the path shorter
        script_dir="$(cd "${script_dir}" || exit 1; pwd)"
else
        # convert to full path and get the directory name of that
        script_name="$(readlink -f "${self_path}")"
        script_dir="$(dirname "${script_name}")"
fi

echo 当前目录为${script_dir}.

if [ -d "${script_dir}/HCppBox" ]
then
	pushd "${script_dir}/HCppBox"
	git pull			
	popd
else
	git clone -b master https://github.com/HEYAHONG/HCppBox.git "${script_dir}/HCppBox" 
fi


if [ -f "${script_dir}/HCppBox/CMakeLists.txt" ]
then
	rm -rf "${script_dir}/master/" 2>/dev/null
	rsync -rl --progress --delete --exclude='.git'  --exclude=".github" --exclude="*.png" --exclude="*.jpg" --exclude="test" --exclude="doc"  --exclude="*.md"  "${script_dir}/HCppBox/" "${script_dir}/master/"
fi 
