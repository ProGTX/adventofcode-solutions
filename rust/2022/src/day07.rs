use aoc::string::NameToId;
use arrayvec::ArrayVec;

// A filesystem is a list of folders and a list of file sizes
// where each folder contains a list of IDs of files and folders.
// The two are differentiated by file IDs starting at a higher number.
type Folder = ArrayVec<usize, 11>;
type Filesystem = (Vec<Folder>, Vec<u32>);
const FILE_ID_START: usize = 1 << 16;

fn parse(filename: &str) -> Filesystem {
    let mut folder_ids = NameToId::new();
    let mut file_ids = NameToId::from(FILE_ID_START);
    let get_root_dir = || {
        let mut path = ArrayVec::<String, 11>::new();
        path.push("/".to_string());
        path
    };
    let mut current_dir = get_root_dir();
    let mut folders = Vec::new();
    folders.push(Folder::default());
    let mut file_sizes = Vec::new();
    let mut current_dir_id = folder_ids.intern(&current_dir[0]);

    for line in aoc::file::read_lines(filename) {
        if let Some((_, command)) = line.split_once("$ ") {
            if let Some((_, dir)) = command.split_once("cd ") {
                current_dir = match dir {
                    "/" => get_root_dir(),
                    ".." => {
                        current_dir.pop();
                        current_dir
                    }
                    dir => {
                        current_dir.push(dir.to_string());
                        current_dir
                    }
                };
                current_dir_id = folder_ids.intern(&current_dir.join("/"));
            } else {
                // ls, handled on following lines
            }
        } else {
            folders.resize(folder_ids.new_len(folders.len()), Folder::default());
            let (first, name) = line.split_once(" ").unwrap();
            let name = name.to_string();
            folders[current_dir_id].push(match first {
                "dir" => {
                    let mut subfolder = current_dir.clone();
                    subfolder.push(name);
                    folder_ids.intern(&subfolder.join("/"))
                }
                size => {
                    let size = size.parse::<u32>().unwrap();
                    let mut filename = current_dir.clone();
                    filename.push(name);
                    let id = file_ids.intern(&filename.join("/"));
                    file_sizes.push(size);
                    id
                }
            });
        }
    }

    return (folders, file_sizes);
}

fn get_folder_size(folders: &Vec<Folder>, file_sizes: &Vec<u32>, folder_id: usize) -> u32 {
    folders[folder_id]
        .iter()
        .map(|&id| {
            if (id >= FILE_ID_START) {
                file_sizes[id - FILE_ID_START]
            } else {
                get_folder_size(folders, file_sizes, id)
            }
        })
        .sum::<u32>()
}

fn solve_case1((folders, file_sizes): &Filesystem) -> u32 {
    (0..(folders.len()))
        .map(|folder_id| get_folder_size(folders, file_sizes, folder_id))
        .filter(|&folder_size| folder_size <= 100000)
        .sum()
}

fn solve_case2((folders, file_sizes): &Filesystem) -> u32 {
    let used_space = get_folder_size(folders, file_sizes, 0);
    let free_space = 70000000 - used_space;
    let min_delete_space = 30000000 - free_space;

    (1..(folders.len()))
        .map(|folder_id| get_folder_size(folders, file_sizes, folder_id))
        .filter(|&folder_size| folder_size >= min_delete_space)
        .min()
        .unwrap()
}

fn main() {
    println!("Part 1");
    let example = parse("day07.example");
    assert_eq!(95437, solve_case1(&example));
    let input = parse("day07.input");
    assert_eq!(1453349, solve_case1(&input));

    println!("Part 2");
    assert_eq!(24933642, solve_case2(&example));
    assert_eq!(2948823, solve_case2(&input));
}
