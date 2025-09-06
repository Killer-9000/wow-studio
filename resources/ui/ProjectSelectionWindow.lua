
window = 
{
    Name = "window"
    Width = 1920,
    Height = 1080,
    Flags = WINDOW_FLAG_BORDERLESS,
    Children = [
        {
            Name = "$PARENT_top_box",
            Type = ELEMENT_TYPE_GROUP,
            Anchor = ELEMENT_ANCHOR_TOP_CENTER,
            Offset = { 0, 0 },
            Size = { 100, 10 },
            Children = [
                {
                    Type = ELEMENT_TYPE_TEXT,
                    Anchor = ELEMENT_ANCHOR_CENTER,
                    Offset = { 0, 0 },
                    Size = { 100, 25},
                    Text = "Project selection",
                    Children = []
                }
            ]
        },
        {
            Name = "$PARENT_bottom_box",
            Type = ELEMENT_TYPE_GROUP,
            Anchor = ELEMENT_ANCHOR_BOTTOM_CENTER,
            Offset = { 0, 0 },
            Size = { 100, 10 },
            Children = [
                {
                    Name = "$PARENT_list_box",
                    Type = ELEMENT_TYPE_GROUP,
                    Anchor = ELEMENT_ANCHOR_LEFT_CENTER,
                    Offset = { 0, 0 },
                    Size = { 75, 100 },
                    Children = [
                        {
                            Name = "$PARENT_list",
                            Type = ELEMENT_TYPE_LIST,
                            Anchor = ELEMENT_ANCHOR_CENTER,
                            Offset = { 0, 0 },
                            Size = { 100, 100 }
                        }
                    ]
                },
                {
                    Name = "$PARENT_options_box",
                    Type = ELEMENT_TYPE_GROUP,
                    Anchor = ELEMENT_ANCHOR_RIGHT_CENTER,
                    Offset = { 0, 0 },
                    Size = { 25, 100 },
                    Children = [
                        {
                            Type = ELEMENT_TYPE_BUTTON,
                            Anchor = ELEMENT_ANCHOR_TOP_CENTER,
                            Offset = { 0, 5 },
                            Size = { 80, 25},
                            Text = "Enter",
                            OnClick = EnterButton_OnClick,
                            Children = []
                        },
                        {
                            Type = ELEMENT_TYPE_BUTTON,
                            Anchor = ELEMENT_ANCHOR_TOP_CENTER,
                            Offset = { 0, 5 },
                            Size = { 80, 25},
                            Text = "Edit",
                            OnClick = EditButton_OnClick,
                            Children = []
                        },
                        {
                            Type = ELEMENT_TYPE_BUTTON,
                            Anchor = ELEMENT_ANCHOR_TOP_CENTER,
                            Offset = { 0, 5 },
                            Size = { 80, 25},
                            Text = "Remove",
                            OnClick = RemoveButton_OnClick,
                            Children = []
                        }
                    ]
                }
            ]
        }
    ]
}

function EnterButton_OnClick(self)
    -- self = The button ui element
    list = _G["window_bottom_box_list_box_list"]
    project_id = list.GetSelectedChildId()

    -- TODO: Proper way to select a project.
    SelectProject(project_id);
end

function EditButton_OnClick(self)
    list = _G["window_bottom_box_list_box_list"]
    project_id = list.GetSelectedChildId()

    -- TODO: Show popup window with edit options.
    EditProject(project_id);
end

function RemoveButton_OnClick(self)
    list = _G["window_bottom_box_list_box_list"]
    project_id = list.GetSelectedChildId()

    -- TODO: Proper way to select a project.
    RemoveProject(project_id);
end

list = _G["window_bottom_box_list_box_list"]
list.ClearChildren()
projects = GetProjects()
for project, i in ipairs(projects) do

    -- Dynamically add projects to list.
    list.AddChild({
        Name = "$PARENT_project_"..i,
        Type = ELEMENT_TYPE_GROUP,
        Anchor = ELEMENT_ANCHOR_TOP_CENTER,
        Offset = { 0, 0 },
        Size = { 100, 10 },
        Children = [
            {
                Type = ELEMENT_TYPE_GROUP,
                Anchor = ELEMENT_ANCHOR_LEFT_CENTER,
                Offset = { 0, 0 },
                Size = { 80, 100 },
                Children = [
                    {
                        Type = ELEMENT_TYPE_TEXT,
                        Anchor = ELEMENT_ANCHOR_TOP_CENTER,
                        Offset = { 0, 0 },
                        Size = { 100, 30},
                        Text = project.project_name
                        Children = []
                    },
                    {
                        Type = ELEMENT_TYPE_TEXT,
                        Anchor = ELEMENT_ANCHOR_BOTTOM_CENTER,
                        Offset = { 0, 0 },
                        Size = { 100, 30},
                        Text = project.project_path
                        Children = []
                    },
                    {
                        Type = ELEMENT_TYPE_TEXT,
                        Anchor = ELEMENT_ANCHOR_BOTTOM_CENTER,
                        Offset = { 0, 0 },
                        Size = { 100, 30},
                        Text = project.client_path
                        Children = []
                    }
                ]
            },
            {
                Type = ELEMENT_TYPE_GROUP,
                Anchor = ELEMENT_ANCHOR_RIGHT_CENTER,
                Offset = { 0, 0 },
                Size = { 20, 100 },
                Children = [
                    {
                        Type = ELEMENT_TYPE_TEXT,
                        Anchor = ELEMENT_ANCHOR_TOP_CENTER,
                        Offset = { 0, 0 },
                        Size = { 100, 30},
                        Text = GetExpansionName(project.client_version)
                        Children = []
                    }
                ]
            }
        ]
    })

end
